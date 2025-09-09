import ctypes
import os

# Load the shared library
lib_path = os.path.join(os.path.dirname(__file__), '..', 'build', 'libprocess_command.so')  # Adjust based on OS
lib = ctypes.CDLL(lib_path)

# Define function signature
lib.process_command.argtypes = [ctypes.c_char_p]  # Input: const char*
lib.process_command.restype = ctypes.c_char_p     # Output: char*

# Call it (convert strings to bytes)
command = b"hello from Python"  # Use bytes for C compatibility
result = lib.process_command(command)
print(result.decode('utf-8'))  # Convert back to string: "ACK: hello from Python"

# If using std::string, ctypes needs careful handling; this assumes char* return if you revert.