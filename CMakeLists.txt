cmake_minimum_required(VERSION 3.10)  # Fine; 3.12+ for better MSVC if possible
project(ProcessCommandModule LANGUAGES CXX)

# Set policy to honor visibility for all targets
cmake_policy(SET CMP0063 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)  # Enforce strict C++14 (no GNU extensions)

add_library(process_command SHARED src/process_command.cpp)

# Includes and properties
target_include_directories(process_command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Visibility: Only for GCC/Clang; skip on MSVC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set_target_properties(process_command PROPERTIES 
        CXX_VISIBILITY_PRESET hidden 
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# Install
install(TARGETS process_command
        LIBRARY DESTINATION lib
        INCLUDES DESTINATION include)  # Hint for imported targets (optional)
install(FILES include/process_command.h DESTINATION include)  # Install public header

# Testing
enable_testing()  # Removed include(CTest)—not needed

# =============================
# Clang-Format Integration
# =============================

find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-14 clang-format-13 clang-format-12)

if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    # Check formatting (fails if not compliant)
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror --style=file -i ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking code formatting with clang-format..."
    )

    # Auto-format all files
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i --style=file ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting code with clang-format..."
    )

    # Optional: Add to test suite
    add_test(NAME clang-format-check
             COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror --style=file -i ${ALL_SOURCE_FILES}
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(WARNING "clang-format not found - 'check-format' and 'format' targets not available")
endif()

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2 # Use the latest stable tag or desired version
)
FetchContent_MakeAvailable(Catch2)
message(STATUS "Catch2 source dir: ${catch2_SOURCE_DIR}")
message(STATUS "Catch2 binary dir: ${catch2_BINARY_DIR}")
add_executable(test_process_command tests/test_process_command.cpp)
target_link_libraries(test_process_command PRIVATE process_command Catch2::Catch2WithMain)
target_include_directories(test_process_command PRIVATE ${CMAKE_BINARY_DIR}/_deps/catch2-src/src)

include(CTest)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(test_process_command)
add_test(NAME ProcessCommandTest COMMAND test_process_command)

