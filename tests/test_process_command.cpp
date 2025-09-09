#include "../include/process_command.h"
#include <iostream>
#include <cassert>

int main()
{
  char *raw_result = process_command("hello");
  std::string result(raw_result);
  free(raw_result);
  assert(result == "ACK: hello"); // Test passes if true
  std::cout << "Test passed: " << result << std::endl;

  raw_result = process_command("");
  result = std::string(raw_result);
  free(raw_result);
  assert(result == "ACK: (null or empty command)");
  std::cout << "Empty test passed." << std::endl;

  return 0;
}