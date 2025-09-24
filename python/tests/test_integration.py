from pathlib import Path

import pytest

from python.examples.example import (
    RETURN_CODE_MESSAGES,
    call_process_command,
    format_output,
    get_lib_filename,
)


# --- Helper: locate the prebuilt library reliably ---
def _find_prebuilt_lib() -> Path:
    """Locate the prebuilt C++ shared library in python/build/."""
    build_dir = Path(__file__).parent.parent / "build"
    lib_name = get_lib_filename()
    lib_path = build_dir / lib_name

    if not lib_path.is_file():
        raise FileNotFoundError(
            f"Prebuilt library not found at {lib_path}. "
            "Ensure the C++ library is built and placed in python/build/."
        )
    return lib_path


# --- Fixture: inject library path WITHOUT global env mutation ---
@pytest.fixture(scope="session")
def lib_path() -> Path:
    """Provide the path to the prebuilt C++ shared library."""
    return _find_prebuilt_lib()


@pytest.fixture(autouse=True)
def _patch_lib_path(monkeypatch, lib_path):
    """Ensure call_process_command uses the prebuilt lib without env vars."""
    # Inject via environment—but safely, per-test, using monkeypatch
    monkeypatch.setenv("PROCESS_COMMAND_BASE_DIR", str(lib_path.parent))
    monkeypatch.setenv("PROCESS_COMMAND_LIB", lib_path.name)


# --- Actual integration tests ---
@pytest.mark.integration
def test_valid_command_returns_ack():
    code, response = call_process_command("PING")
    assert code == 0
    assert response == "ACK: PING"


@pytest.mark.integration
def test_invalid_command_returns_error_code():
    code, response = call_process_command("INVALID")
    assert code == -4
    assert response is None

    message = format_output(
        code, response, no_emoji=True, messages=RETURN_CODE_MESSAGES
    )
    assert "invalid command" in message


@pytest.mark.integration
def test_buffer_too_small_reports_overflow():
    # Use a buffer smaller than the expected ACK response for COMMAND_2
    code, response = call_process_command("PINGI", bufsize=7)
    assert code == -3
    assert response is None

    message = format_output(
        code, response, no_emoji=True, messages=RETURN_CODE_MESSAGES
    )
    assert "does not fit" in message


# --- Optional: sanity check that error codes match the INI file ---
def test_error_code_messages_match_ini():
    """Ensure Python loaded the expected error messages."""
    assert RETURN_CODE_MESSAGES[0] == "Success"
    assert RETURN_CODE_MESSAGES[-4] == "NACK written successfully (invalid command)"
    assert RETURN_CODE_MESSAGES[-3] == "ACK response does not fit in buffer"
