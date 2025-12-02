cmake_minimum_required(VERSION 3.10)
project(TCPScanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcp_scanner main.cpp)

enable_testing()
add_test(NAME basic_compilation_test COMMAND tcp_scanner 127.0.0.1 22 22)