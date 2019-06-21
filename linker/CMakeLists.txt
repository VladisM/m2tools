cmake_minimum_required(VERSION 3.2.0)
project(linker C)

set(sources linker.c)

add_executable(${target_prefix}-linker ${sources})

target_link_libraries(${target_prefix}-linker objlib isalib sllib ldmlib)

target_compile_definitions(${target_prefix}-linker PRIVATE -DPROG_NAME="${target_prefix}-linker")
