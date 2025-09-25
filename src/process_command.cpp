#include "process_command.h"

#include <algorithm>
#include <cstring>
#include <string>

namespace detail {
/*  external-linkage definitions  */
const char kPrefix[] = "ACK: ";
const char kNackPrefix[] = "NACK: ";
const char kErrorMsg[] = "(null or empty command)";
const char kInvalidCmdMsg[] = "Invalid command";
const std::array<const char *, 3> kValidCommands = {"PING", "STATUS", "AUTH"};
}  // namespace detail

namespace {

bool is_valid_command(const char *cmd) {
    if (!cmd || cmd[0] == '\0') return false;
    auto eq = [](const char *a, const char *b) { return std::strcmp(a, b) == 0; };
    return std::any_of(detail::kValidCommands.begin(), detail::kValidCommands.end(),
                       [cmd, eq](const char *v) { return eq(cmd, v); });
}

bool build_and_copy(const char *prefix, const char *msg, char *buf, size_t bufsize) {
    std::string out(prefix);
    out += msg;
    if (out.size() + 1 > bufsize) {  // +1 for NUL
        if (bufsize) buf[0] = '\0';
        return false;
    }
    std::strcpy(buf, out.c_str());
    return true;
}

}  // anonymous namespace

/*  C interface  */
extern "C" int process_command(const char *command, char *buffer, size_t bufsize) {
    /* 1.  absolutely no write when buffer unusable */
    if (!buffer || bufsize == 0) return -1;
    if (bufsize > 0) buffer[0] = '\0';

    /* 2.  null / empty command */
    if (!command || command[0] == '\0') {
        return build_and_copy(detail::kPrefix, detail::kErrorMsg, buffer, bufsize) ? 0 : -2;
    }

    /* 3.  unknown command */
    if (!is_valid_command(command)) {
        bool ok = build_and_copy(detail::kNackPrefix, detail::kInvalidCmdMsg, buffer, bufsize);
        return ok ? -4 : -5;
    }

    /* 4.  valid command */
    return build_and_copy(detail::kPrefix, command, buffer, bufsize) ? 0 : -3;
}