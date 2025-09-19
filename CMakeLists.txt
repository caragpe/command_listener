cmake_minimum_required(VERSION 3.10)
project(ProcessCommandModule LANGUAGES CXX)

# Set Modern CMake Policies
cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0063 NEW)

# C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Shared Library
add_library(process_command SHARED src/process_command.cpp)

# Mark header as public
set_target_properties(process_command PROPERTIES
    PUBLIC_HEADER include/process_command.h
)

# Public include directory for consumers
target_include_directories(process_command
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Hide symbols by default (GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set_target_properties(process_command PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# Install Rules
include(GNUInstallDirs)

install(TARGETS process_command
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Header installed via PUBLIC_HEADER above

install(FILES include/process_command.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Testing Setup
enable_testing()

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


# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Build test executable
add_executable(test_process_command tests/test_process_command.cpp)

# Link against your library + Catch2
target_link_libraries(test_process_command
    PRIVATE
        process_command
        Catch2::Catch2WithMain
)

# Discover and register tests (Catch2 v3)
include(CTest)
include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(test_process_command)