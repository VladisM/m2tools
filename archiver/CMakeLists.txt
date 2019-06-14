cmake_minimum_required(VERSION 3.2.0)
project(archiver C)

set(sources archiver.c)

add_executable(${target_prefix}-archiver ${sources})

target_link_libraries(${target_prefix}-archiver objlib sllib)

target_compile_definitions(${target_prefix}-archiver PRIVATE -DPROG_NAME="${target_prefix}-archiver")
