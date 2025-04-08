cmake_minimum_required(VERSION 3.14)
project(mp3lite_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wconversion")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-default -Wwrite-strings")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wshadow -Wfloat-equal")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wcast-qual -Wcast-align")

enable_testing()

add_subdirectory(unit_tests)