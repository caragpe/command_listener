#include "process_command.h"
#include <string>

std::string process_command(const std::string &command)
{
  if (command.empty())
  {
    return "ACK: (empty command)";
  }
  return "ACK: " + command;
}