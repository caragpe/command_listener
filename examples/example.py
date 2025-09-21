import ctypes
import os
import platform
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

RETURN_CODE_MESSAGES = {
    0: "Success",
    -1: "Invalid buffer or buffer size is zero",
    -2: "Null/empty command message does not fit in buffer",
    -3: "ACK response does not fit in buffer",
    -4: "NACK written successfully (invalid command)",
    -5: "NACK response does not fit in buffer",
}


def get_lib_filename():
    system = platform.system()
    return {
        "Windows": "process_command.dll",
        "Darwin": "libprocess_command.dylib",
    }.get(system, "libprocess_command.so")


# Load the shared library
lib_path = os.path.join(
    os.path.dirname(__file__), "..", "build", "libprocess_command.so"
)
lib = ctypes.CDLL(lib_path)

# Define function signature: Returns int (0 success, -1 error).
# The function writes to buffer.
lib.process_command.argtypes = [
    ctypes.c_char_p,
    ctypes.c_char_p,
    ctypes.c_size_t,
]
lib.process_command.restype = ctypes.c_int

# Input as bytes
command = b"COMMAND_1"

# Pre-allocate buffer (choose size based on max expected response; e.g., 1024)
BUF_SIZE = 1024
# create_string_buffer equivalent; empty array
buffer = (ctypes.c_char * BUF_SIZE)()

# Call the function
result_code = lib.process_command(command, buffer, BUF_SIZE)

if result_code == 0:
    # Convert buffer to Python string (null-terminated)
    response = ctypes.string_at(buffer).decode("utf-8")
    print(response)  # "ACK: hello from Python"
else:
    print("Error: Function failed (e.g., buffer overflow or invalid input)")
