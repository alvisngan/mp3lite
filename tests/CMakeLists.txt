cmake_minimum_required(VERSION 3.14)
project(mp3lite_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED 99)

enable_testing()

add_subdirectory(unit_tests)