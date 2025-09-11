#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../include/process_command.h"

#include <cstring>
#include <string>

// Helper function to run process_command and check results
void test_command(const char *input, size_t bufsize, const std::string &expected_output, int expected_code, const std::string &test_name)
{
  char buffer[bufsize];
  std::memset(buffer, 0, bufsize); // Ensure clean buffer
  int result_code = process_command(input, buffer, bufsize);
  REQUIRE(result_code == expected_code);                      // Check return code
  REQUIRE(std::strcmp(buffer, expected_output.c_str()) == 0); // Check buffer content
}

TEST_CASE("process_command functionality", "[process_command]")
{
  SECTION("Normal input")
  {
    test_command("hello", 1024, "ACK: hello", 0, "Normal input");
  }

  SECTION("Empty input")
  {
    test_command("", 1024, "ACK: (null or empty command)", 0, "Empty input");
  }

  SECTION("Null input")
  {
    test_command(nullptr, 1024, "ACK: (null or empty command)", 0, "Null input");
  }

  SECTION("Small buffer")
  {
    test_command("hello", 5, "", -1, "Small buffer");
  }
}