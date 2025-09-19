#define CATCH_CONFIG_MAIN
#include "../include/process_command.h"
#include <catch2/catch_all.hpp>
#include <cstring>
#include <string>

namespace {
constexpr int kSuccess = 0;           // Successful execution
constexpr int kInvalidBuffer = -1;    // Null or zero-size buffer
constexpr int kErrorMsgOverflow = -2; // Buffer too small for error message
constexpr int kCommandOverflow = -3;  // Buffer too small for command
} // namespace

// Helper function to run process_command and check results
void test_command(const char *input, size_t bufsize,
                  const std::string &expected_output, int expected_code,
                  const std::string &test_name) {
  char buffer[bufsize];
  std::memset(buffer, 0, bufsize); // Ensure clean buffer
  int result_code = process_command(input, buffer, bufsize);
  REQUIRE(result_code == expected_code);
  REQUIRE(std::strcmp(buffer, expected_output.c_str()) == 0);
}

TEST_CASE("process_command functionality", "[process_command]") {
  SECTION("Normal input") {
    test_command("hello", 1024, "ACK: hello", kSuccess, "Normal input");
  }

  SECTION("Empty input") {
    test_command("", 1024, "ACK: (null or empty command)", kSuccess,
                 "Empty input");
  }

  SECTION("Null input") {
    test_command(nullptr, 1024, "ACK: (null or empty command)", kSuccess,
                 "Null input");
  }

  SECTION("Small buffer") {
    test_command("hello", 5, "", kCommandOverflow, "Small buffer");
  }

  SECTION("Invalid buffer") {
    char *null_buffer = nullptr;
    int result_code = process_command("hello", null_buffer, 0);
    REQUIRE(result_code == kInvalidBuffer);
  }
}