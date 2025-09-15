#include "process_command.h"
#include <cstddef> // For size_t, nullptr
#include <cstring> // For strlen, strcpy, strcat, strncpy

namespace {
constexpr const char *kPrefix = "ACK: ";
constexpr const char *kNackPrefix = "NACK: ";
constexpr const char *kErrorMsg = "(null or empty command)";
constexpr const char *kInvalidCmdMsg = "Invalid command";
} // namespace

// Writes to buffer with prefix and message, ensuring null-termination
// Returns true if successful, false if buffer too small
bool write_to_buffer(const char *message, size_t message_len, char *buffer,
                     size_t bufsize) {
  const size_t total_len =
      kPrefixLen + message_len + 1; // +1 for null terminator
  if (total_len > bufsize) {
    buffer[0] = '\0'; // Clear buffer on overflow
    return false;
  }
  strcpy(buffer, kPrefix);
  strcat(buffer, message);
  return true;
}

extern "C" int process_command(const char *command, char *buffer,
                               size_t bufsize) {
  // Validate inputs
  if (!buffer || bufsize == 0) {
    return -1; // Invalid buffer
  }

  // Clear buffer to ensure null-termination on error
  buffer[0] = '\0';

  // Handle null or empty command
  if (!command || command[0] == '\0') {
    return write_to_buffer(kErrorMsg, kErrorMsgLen, buffer, bufsize) ? 0 : -2;
  }

  // Validate command length
  size_t cmd_len = strlen(command);
  return write_to_buffer(command, cmd_len, buffer, bufsize) ? 0 : -3;
}