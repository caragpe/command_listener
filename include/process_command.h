#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

extern "C"
{
  char *process_command(const char *command) __attribute__((visibility("default")));
  // The returned pointer must be freed by the caller using free() to avoid memory leaks.
}

#endif // PROCESS_COMMAND_H