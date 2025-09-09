#include "process_command.h"
#include <cstring> // For strlen, strcpy, strcat, strdup
#include <cstdlib> // For malloc, strdup

extern "C" char *process_command(const char *command)
{
  // Handle null or empty input
  if (!command || command[0] == '\0')
  {
    return strdup("ACK: (null or empty command)"); // Allocates and returns; caller must free
  }
  size_t len = strlen(command);
  constexpr size_t fixed_len = 5; // "ACK: " is 5 chars
  char *response = (char *)malloc(len + fixed_len + 1);
  if (response == nullptr)
  {
    return strdup("ACK: (allocation failed)"); // Out-of-memory
  }
  strcpy(response, "ACK: ");
  strcat(response, command);
  // Null terminator is automatically added by strcpy/strcat due to allocation +1
  return response;
}