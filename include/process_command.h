#ifndef PROCESS_COMMAND_H
#define PROCESS_COMMAND_H

#include <array>
#include <cstddef>  // size_t

#ifdef _WIN32
#define PROCESS_COMMAND_EXPORT __declspec(dllexport)
#else
#define PROCESS_COMMAND_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
//  return-code summary
//   0  success (ACK or null-cmd message written)
//  -1  invalid buffer (nullptr or zero size)
//  -2  null-cmd message does not fit
//  -3  ACK message does not fit
//  -4  NACK written successfully
//  -5  NACK message does not fit
PROCESS_COMMAND_EXPORT int process_command(const char *command, char *buffer, size_t bufsize);
}

/*  declarations only  */
namespace detail {
extern const char kPrefix[];
extern const char kNackPrefix[];
extern const char kErrorMsg[];
extern const char kInvalidCmdMsg[];
extern const std::array<const char *, 3> kValidCommands;
}  // namespace detail

#endif  // PROCESS_COMMAND_H