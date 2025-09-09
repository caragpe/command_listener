cmake_minimum_required(VERSION 3.10)
project(ProcessCommandModule LANGUAGES CXX)

# Set policy to honor visibility for all targets
cmake_policy(SET CMP0063 NEW)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(process_command SHARED src/process_command.cpp)

# Includes and properties
target_include_directories(process_command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Visibility: Only for GCC/Clang; skip on MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set_target_properties(process_command PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
endif()

# Public header property (for install)
set_target_properties(process_command PROPERTIES PUBLIC_HEADER include/process_command.h)

# Install
install(TARGETS process_command
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

# Testing
enable_testing()
add_executable(test_process_command tests/test_process_command.cpp)
target_link_libraries(test_process_command process_command)
target_include_directories(test_process_command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_test(NAME ProcessCommandTest COMMAND test_process_command)