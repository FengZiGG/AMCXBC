cmake_minimum_required(VERSION 3.14)
project(AMCX_BC)

set(CMAKE_CXX_STANDARD 14)

add_executable(AMCX_BC src/main.cpp src/solverconf.h src/util.cpp src/util.h src/constants.h)

SET(CMAKE_CXX_FLAGS "$ENV{CFLAGS} -O3 -Wall")

target_link_libraries(AMCX_BC cryptominisat5)