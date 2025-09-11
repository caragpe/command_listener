#include "process_command.h"
#include <cstring> // For strlen, strcpy, strcat, strncpy
#include <cstdlib> // For NULL (if needed)

extern "C" int process_command(const char *command, char *buffer, size_t bufsize)
{
  if (bufsize == 0)
    return -1; // Can't write to zero-size buffer

  const char *prefix = "ACK: ";
  size_t prefix_len = 5;      // strlen("ACK: ")
  size_t avail = bufsize - 1; // Reserve 1 for null terminator

  if (!command || command[0] == '\0')
  {
    // Error case: Write to buffer if space allows
    const char *err_msg = "(null or empty command)";
    size_t err_len = strlen(err_msg);
    if (prefix_len + err_len + 1 <= avail)
    {
      strcpy(buffer, "ACK: ");
      strcat(buffer, err_msg);
      buffer[bufsize - 1] = '\0'; // Ensure null-terminated
      return 0;
    }
    else
    {
      buffer[0] = '\0'; // Empty on overflow
      return -1;
    }
  }

  size_t cmd_len = strlen(command);
  if (prefix_len + cmd_len + 1 > avail)
  {
    buffer[0] = '\0'; // Truncate on overflow
    return -1;
  }

  strcpy(buffer, prefix);
  strcat(buffer, command);
  buffer[bufsize - 1] = '\0'; // Ensure null-terminated
  return 0;                   // Success
}