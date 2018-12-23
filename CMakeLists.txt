cmake_minimum_required(VERSION 3.13)
project(ComplilingTest C)

set(CMAKE_C_STANDARD 99)

add_executable(Scan scan.c)
add_executable(Generator generator_LR_0.c stack.c stack.h)
target_link_libraries(Scan m)