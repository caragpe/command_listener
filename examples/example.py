import ctypes
import os

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..',
                        'build', 'libprocess_command.so')
lib = ctypes.CDLL(lib_path)

# Define function signature: Returns int (0 success, -1 error), writes to buffer
lib.process_command.argtypes = [
    ctypes.c_char_p, ctypes.c_char_p, ctypes.c_size_t]
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
    response = ctypes.string_at(buffer).decode('utf-8')
    print(response)  # "ACK: hello from Python"
else:
    print("Error: Function failed (e.g., buffer overflow or invalid input)")
