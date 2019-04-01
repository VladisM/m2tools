cmake_minimum_required(VERSION 3.2.0)
project(assembler C)

set(sources file_gen.c main.c pass1.c pass2.c symbol_table.c tokenizer.c util.c)

add_executable(${target_prefix}-assembler ${sources})

target_link_libraries(${target_prefix}-assembler objlib isalib)

target_compile_definitions(${target_prefix}-assembler PRIVATE -DPROG_NAME="${target_prefix}-assembler")
