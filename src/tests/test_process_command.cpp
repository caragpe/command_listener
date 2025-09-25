#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <array>
#include <catch2/catch_all.hpp>
#include <cstring>
#include <string>
#include <vector>

#include "process_command.h"

// Hardcoded literals matching implementation
constexpr const char *kPrefix = "ACK: ";
constexpr const char *kNackPrefix = "NACK: ";
constexpr const char *kErrorMsg = "(null or empty command)";
constexpr const char *kInvalidCmdMsg = "Invalid command";
constexpr std::array<const char *, 3> kValidCommands = {"PING", "STATUS", "AUTH"};

namespace {
/*  return codes that match the implementation  */
constexpr int kSuccess = 0;
constexpr int kInvalidBuffer = -1;
constexpr int kNullCmdOverflow = -2;  // null-msg does not fit
constexpr int kAckOverflow = -3;      // ACK does not fit
constexpr int kNackOk = -4;           // NACK written successfully
constexpr int kNackOverflow = -5;     // NACK does not fit
}  // namespace

/* ---------- helper: verify guard bytes ---------- */
static void check_guards(const std::vector<char> &guarded, size_t bufsize, size_t guard_size) {
    for (size_t i = 0; i < guard_size; ++i) {
        REQUIRE(guarded[i] == char(0xAA));
        REQUIRE(guarded[bufsize + guard_size + i] == char(0xAA));
    }
}

/* ---------- generic test driver ---------- */
static void test_command_impl(const char *cmd, size_t bufsize, const std::string &expected_output,
                              int expected_code, const std::string &test_name, bool use_guards) {
    constexpr size_t GUARD_SIZE = 8;
    std::vector<char> guarded_buffer;
    char *buffer = nullptr;

    if (use_guards) {
        guarded_buffer.resize(bufsize + 2 * GUARD_SIZE, char(0xAA));
        buffer = guarded_buffer.data() + GUARD_SIZE;
    } else {
        guarded_buffer.resize(bufsize);
        buffer = guarded_buffer.data();
    }

    std::memset(buffer, 0x5A, bufsize);  // poison
    int rc = process_command(cmd, buffer, bufsize);

    if (use_guards) check_guards(guarded_buffer, bufsize, GUARD_SIZE);

    REQUIRE(rc == expected_code);

    if (rc == kSuccess || rc == kNackOk) {  // we expect output
        REQUIRE(std::string(buffer) == expected_output);
    } else {  // any failure -> empty
        REQUIRE(buffer[0] == '\0');
    }
}

/* ---------- user-facing helpers ---------- */
static void test_command(const char *cmd, size_t bufsize, const std::string &expected, int code,
                         const std::string &name) {
    test_command_impl(cmd, bufsize, expected, code, name, false);
}

static void test_command_with_guards(const char *cmd, size_t bufsize, const std::string &expected,
                                     int code, const std::string &name) {
    test_command_impl(cmd, bufsize, expected, code, name, true);
}

/* ==============================================================
 *  TEST SUITE
 * ============================================================== */
TEST_CASE("process_command", "[process_command]") {
    SECTION("Valid commands") {
        for (const char *cmd : kValidCommands) {
            std::string expected = std::string(kPrefix) + cmd;
            test_command_with_guards(cmd, 1024, expected, kSuccess, cmd);
        }
    }

    SECTION("Invalid command") {
        test_command_with_guards("INVALID", 1024, std::string(kNackPrefix) + kInvalidCmdMsg,
                                 kNackOk, "invalid");
    }

    SECTION("Leading/trailing space") {
        test_command_with_guards(" PING", 1024, std::string(kNackPrefix) + kInvalidCmdMsg, kNackOk,
                                 "leading space");
        test_command_with_guards("PING ", 1024, std::string(kNackPrefix) + kInvalidCmdMsg, kNackOk,
                                 "trailing space");
    }

    SECTION("Very long invalid command") {
        std::string big(10'000, 'X');
        test_command_with_guards(big.c_str(), 1024, std::string(kNackPrefix) + kInvalidCmdMsg,
                                 kNackOk, "long invalid");
    }

    SECTION("Very long valid command – should still be rejected quickly") {
        std::string big = "PING" + std::string(10'000, 'Y');
        test_command_with_guards(big.c_str(), 1024, std::string(kNackPrefix) + kInvalidCmdMsg,
                                 kNackOk, "long valid prefix");
    }

    SECTION("Null or empty input") {
        test_command_with_guards("", 1024, std::string(kPrefix) + kErrorMsg, kSuccess, "empty");
        test_command_with_guards(nullptr, 1024, std::string(kPrefix) + kErrorMsg, kSuccess, "null");
    }

    SECTION("Exact buffer fit – ACK") {
        const char *cmd = "PING";
        size_t need = std::strlen(kPrefix) + std::strlen(cmd) + 1;
        std::string exp = std::string(kPrefix) + cmd;
        test_command_with_guards(cmd, need, exp, kSuccess, "exact ack");
    }

    SECTION("Exact buffer fit – NACK") {
        const char *cmd = "INVALID";
        size_t need = std::strlen(kNackPrefix) + std::strlen(kInvalidCmdMsg) + 1;
        std::string exp = std::string(kNackPrefix) + kInvalidCmdMsg;
        test_command_with_guards(cmd, need, exp, kNackOk, "exact nack");
    }

    SECTION("Buffer too small by one byte – ACK") {
        const char *cmd = "PING";
        size_t small = std::strlen(kPrefix) + std::strlen(cmd);  // no NUL
        test_command_with_guards(cmd, small, "", kAckOverflow, "ack-1");
    }

    SECTION("Buffer too small by one byte – NACK") {
        size_t small = std::strlen(kNackPrefix) + std::strlen(kInvalidCmdMsg);  // no NUL
        test_command_with_guards("INVALID", small, "", kNackOverflow, "nack-1");
    }

    SECTION("Buffer too small for null-cmd message") {
        size_t small = std::strlen(kPrefix) + std::strlen(kErrorMsg);  // no NUL
        test_command_with_guards(nullptr, small, "", kNullCmdOverflow, "null-1");
    }

    SECTION("Buffer size 1 – valid command") {
        test_command_with_guards("PING", 1, "", kAckOverflow, "size1-valid");
    }

    SECTION("Buffer size 1 – null command") {
        test_command_with_guards(nullptr, 1, "", kNullCmdOverflow, "size1-null");
    }

    SECTION("Zero-size buffer") {
        char dummy = static_cast<char>(0xFF);
        int rc = process_command("PING", &dummy, 0);
        REQUIRE(static_cast<unsigned char>(dummy) == 0xFF);
        REQUIRE(rc == kInvalidBuffer);
    }

    SECTION("nullptr buffer") {
        int rc = process_command("PING", nullptr, 0);
        REQUIRE(rc == kInvalidBuffer);
    }

    SECTION("All valid commands – exact fit loop") {
        for (const char *cmd : kValidCommands) {
            size_t need = std::strlen(kPrefix) + std::strlen(cmd) + 1;
            std::string exp = std::string(kPrefix) + cmd;
            test_command(cmd, need, exp, kSuccess, std::string("exact-") + cmd);
        }
    }
}