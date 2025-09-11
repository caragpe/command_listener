#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

#include <stddef.h> // For size_t

#ifdef _WIN32
#define PROCESS_COMMAND_EXPORT __declspec(dllexport)
#else
#define PROCESS_COMMAND_EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
  // Returns 0 on success, -1 on error (e.g., input too long for buffer or null input)
  // Writes null-terminated response to buffer (up to bufsize-1 chars)
  PROCESS_COMMAND_EXPORT int process_command(const char *command, char *buffer, size_t bufsize);
  // Note: No free needed—caller provides buffer.
}

#endif // PROCESS_COMMAND_H