#include "../include/process_command.h"
#include <iostream>
#include <cassert> // For assertions

int main()
{
  std::string result = process_command("hello");
  assert(result == "ACK: hello"); // Test passes if true
  std::cout << "Test passed: " << result << std::endl;

  result = process_command("");
  assert(result == "ACK: (null or empty command)");
  std::cout << "Empty test passed." << std::endl;

  return 0;
}