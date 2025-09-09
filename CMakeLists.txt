cmake_minimum_required(VERSION 3.10)
project(ProcessCommandModule)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(process_command SHARED src/process_command.cpp)

target_include_directories(process_command PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS process_command DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)