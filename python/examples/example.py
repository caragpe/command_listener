import ctypes
import os
import platform
import logging
import argparse
import configparser
from pathlib import Path
from typing import Union, Tuple, Optional

logger = logging.getLogger(__name__)


def init_logging(debug: bool) -> None:
    """Initialize logging once (idempotent) with sensible defaults.

    If the root logger already has handlers (configured by host app), this
    function does nothing except optionally raise the module logger level.
    """
    root = logging.getLogger()
    if not root.handlers:  # configure only if nothing configured yet
        level = logging.DEBUG if debug else logging.INFO
        logging.basicConfig(
            level=level,
            format="%(asctime)s %(levelname)s %(name)s: %(message)s",
            datefmt="%H:%M:%S",
        )
    if debug:
        logger.setLevel(logging.DEBUG)


def load_return_code_messages(config_path: str | None = None) -> dict[int, str]:
    """Load error code messages from INI or return defaults.

    Resolution order (first existing, readable file wins):
      1. Explicit path passed in (as-is; relative resolved against CWD)
      2. error_codes.ini located next to this script
      3. error_codes.ini in the current working directory
    """
    candidates: list[Path] = []
    if config_path:
        p = Path(config_path)
        candidates.append(p if p.is_absolute() else Path.cwd() / p)
    script_dir_file = Path(__file__).with_name("error_codes.ini")
    candidates.append(script_dir_file)
    candidates.append(Path.cwd() / "error_codes.ini")

    config = configparser.ConfigParser()
    for c in candidates:
        try:
            if c.exists():
                config.read(c)
                if config.sections():
                    logger.debug("Loaded error code messages from %s", c)
                    return {
                        int(k): v
                        for section in config.sections()
                        for k, v in config.items(section)
                    }
        except OSError:
            continue

    # Defaults
    return {
        0: "Success",
        -1: "Invalid buffer or buffer size is zero",
        -2: "Null/empty command message does not fit in buffer",
        -3: "ACK response does not fit in buffer",
        -4: "NACK written successfully (invalid command)",
        -5: "NACK response does not fit in buffer",
    }


RETURN_CODE_MESSAGES = load_return_code_messages()


def get_lib_filename() -> str:
    """Determine the shared library filename based on platform and architecture.

    Returns:
        str: The appropriate library filename.

    Raises:
        ValueError: If the platform is unsupported.
    """
    system = platform.system()
    machine = platform.machine()
    if system == "Windows":
        return "process_command.dll"
    elif system == "Darwin":
        return "libprocess_command.dylib"
    elif system == "Linux":
        if "arm" in machine or "aarch" in machine:
            return "libprocess_command_arm.so"
        return "libprocess_command.so"
    else:
        raise ValueError(
            (
                f"Unsupported platform: {system} ({machine}). "
                "Please provide a custom library path."
            )
        )


def encode_string(text: str, encoding: str, context: str = "command") -> bytes:
    """Encode a string with error handling.

    Args:
        text (str): The string to encode.
        encoding (str): The encoding to use.
        context (str): Description of the string (for error messages).
            Defaults to 'command'.

    Returns:
        bytes: The encoded string.

    Raises:
        ValueError: If encoding fails.
    """
    try:
        return text.encode(encoding)
    except UnicodeEncodeError as e:
        raise ValueError(
            f"Failed to encode {context} with '{encoding}': {e}") from e


def decode_bytes(data: ctypes.Array, size: int, encoding: str) -> str:
    """Decode a byte buffer with error handling, stripping null bytes.

    Args:
        data (ctypes.Array): The byte buffer to decode.
        size (int): The size of the buffer.
        encoding (str): The encoding to use.

    Returns:
        str: The decoded string with null bytes removed.
    """
    try:
        return ctypes.string_at(data, size).decode(
            encoding, errors='replace').rstrip('\x00')
    except UnicodeDecodeError as e:
        logger.warning(
            f"Failed to decode response with '{encoding}': {e}. "
            "Using replacement characters."
        )
        return ctypes.string_at(data, size).decode(
            encoding, errors='replace').rstrip('\x00')


def call_process_command(command: str,
                         lib_path: Optional[Union[str,
                                                  Path]] = None,
                         bufsize: int = 4096,
                         encoding: str = 'utf-8') -> Tuple[int,
                                                           Optional[str]]:
    """Call a C++ binary's process_command function via ctypes.

    Args:
        command (str): The command string to send to the C++ binary.
        lib_path (str | Path, optional): Path to the shared library.
            Defaults to None (uses env var or default).
        bufsize (int, optional): Size of the response buffer. Defaults to 4096.
        encoding (str, optional): Encoding for command/response. Defaults to 'utf-8'.

    Returns:
        Tuple[int, Optional[str]]: Return code from the C++ binary and the
            response string (or None on error).

    Raises:
        ValueError: If command is empty or bufsize is invalid.
        FileNotFoundError: If the library file does not exist.
        OSError: If the library fails to load.
    """
    if not command:
        raise ValueError("Command cannot be empty or None.")

    if lib_path is None:
        lib_name = os.getenv("PROCESS_COMMAND_LIB", get_lib_filename())
        base_dir = os.getenv("PROCESS_COMMAND_BASE_DIR",
                             Path(__file__).parent.parent / 'build')
        lib_path = Path(base_dir) / lib_name

    lib_path = Path(lib_path)
    if not lib_path.exists():
        raise FileNotFoundError(
            (
                f"Library not found at '{lib_path}'. "
                "Check path or set PROCESS_COMMAND_LIB env var."
            )
        )

    try:
        lib = ctypes.CDLL(str(lib_path))
    except OSError as e:
        raise OSError(f"Failed to load library '{lib_path}': {e}") from e

    lib.process_command.argtypes = [
        ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
    lib.process_command.restype = ctypes.c_int

    command_bytes = encode_string(command, encoding)

    if bufsize < 1:
        raise ValueError("Buffer size must be at least 1.")

    buffer = (ctypes.c_char * bufsize)()
    result_code = lib.process_command(command_bytes, buffer, bufsize)

    response = None
    if result_code >= 0:
        response = decode_bytes(buffer, bufsize, encoding)
        if len(response.encode(encoding)) >= bufsize - 1:
            logger.warning(
                "Response may be truncated; consider increasing buffer size.")

    msg = RETURN_CODE_MESSAGES.get(
        result_code, f"Unknown error ({result_code})")
    logger.debug("Result code: %d, Message: %s, Response: %s",
                 result_code, msg, response)

    return result_code, response


def format_output(code: int,
                  response: Optional[str],
                  no_emoji: bool,
                  messages: dict[int,
                                 str]) -> str:
    """Format the output for display, with optional emojis.

    Args:
        code (int): The return code from the C++ binary.
        response (Optional[str]): The response string, if any.
        no_emoji (bool): If True, disable emoji output.
        messages (dict[int, str]): Mapping of return codes to messages.

    Returns:
        str: Formatted output string.
    """
    emoji_success = "✅ " if not no_emoji else ""
    emoji_error = "❌ " if not no_emoji else ""
    emoji_note = "📝 " if not no_emoji else ""

    if code == 0:
        return f"{emoji_success}{response}"
    msg = messages.get(code, 'Unknown')
    output = f"{emoji_error}Error {code}: {msg}"
    if response:
        output += f"\n{emoji_note}{response}"
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Call C++ process_command library.")
    parser.add_argument("command", help="Command to send (e.g., PING)")
    parser.add_argument(
        "--lib-path", help="Path to the shared library (overrides env var)")
    parser.add_argument("--bufsize", type=int, default=4096,
                        help="Buffer size (default: 4096)")
    parser.add_argument("--encoding", default="utf-8",
                        help="Encoding for command/response (default: utf-8)")
    parser.add_argument("--debug", action="store_true",
                        help="Enable debug logging")
    parser.add_argument("--no-emoji", action="store_true",
                        help="Disable emoji in output for plain text")

    args = parser.parse_args()

    init_logging(args.debug)
    if args.debug:
        logger.debug("Debug mode enabled")

    try:
        code, resp = call_process_command(
            args.command,
            lib_path=args.lib_path,
            bufsize=args.bufsize,
            encoding=args.encoding
        )
        print(format_output(
            code,
            resp,
            args.no_emoji,
            RETURN_CODE_MESSAGES
        ))
    except (ValueError, FileNotFoundError, OSError, UnicodeEncodeError) as e:
        logger.error(f"Unexpected error: {e}")
        print(f"❌ Fatal error: {e}")
