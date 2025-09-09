#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

extern "C"
{
  char *process_command(const char *command);
  // The returned pointer must be freed by the caller using free() to avoid memory leaks.
}

#endif // PROCESS_COMMAND_H