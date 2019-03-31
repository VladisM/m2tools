cmake_minimum_required(VERSION 3.2.0)
project(m2tools C)
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_STATIC_LIBS ON)
set(target_prefix "m2")

#define libraries targets
add_subdirectory(./lib-isa/)
add_subdirectory(./lib-obj/)
add_subdirectory(./lib-microtar/)
add_subdirectory(./lib-sl/)

#define executable targets
add_executable(${target_prefix}-assembler
    assembler/file_gen.c
    assembler/main.c
    assembler/pass1.c
    assembler/pass2.c
    assembler/symbol_table.c
    assembler/tokenizer.c
    assembler/util.c
)

add_executable(${target_prefix}-objread
    objread/main.c
)

add_executable(${target_prefix}-archiver
    archiver/main.c
)

#define libraries dep for executables
target_link_libraries(m2-assembler
    objlib
    isalib
)

target_link_libraries(m2-objread
    objlib
    isalib
)

target_link_libraries(m2-archiver
    objlib
    sllib
)

add_definitions(-DVERSION="v1.0 Alpha")
add_definitions(-DTARGET_ARCH_NAME="MARK-II")

target_compile_definitions(${target_prefix}-assembler PRIVATE -DPROG_NAME="${target_prefix}-assembler")
target_compile_definitions(${target_prefix}-objread   PRIVATE -DPROG_NAME="${target_prefix}-objread")
target_compile_definitions(${target_prefix}-archiver  PRIVATE -DPROG_NAME="${target_prefix}-archiver")
