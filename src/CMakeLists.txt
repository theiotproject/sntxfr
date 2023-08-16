cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_C_STANDARD 17)

include_directories("../include")

add_executable(sntxfr main.c test.c)
