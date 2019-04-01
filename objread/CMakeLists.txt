cmake_minimum_required(VERSION 3.2.0)
project(objread C)

set(sources main.c)

add_executable(${target_prefix}-objread ${sources})

target_link_libraries(${target_prefix}-objread objlib isalib)

target_compile_definitions(${target_prefix}-objread PRIVATE -DPROG_NAME="${target_prefix}-objread")
