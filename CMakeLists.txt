cmake_minimum_required(VERSION 3.2.0)
project(m2tools C)
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_STATIC_LIBS ON)

#define executable targets
add_executable(m2-ldm2mif ldm2mif/main.c)
add_executable(m2-loader loader/main.c loader/sender.c)
add_executable(m2-assembler assembler/file_gen.c assembler/main.c assembler/pass1.c assembler/pass2.c assembler/symbol_table.c assembler/tokenizer.c assembler/util.c)
add_executable(m2-objread objread/main.c)
add_executable(m2-archiver archiver/main.c)

#test targets
add_executable(objlib_test test_progs/objlib_test.c)
add_executable(sllib_test test_progs/sllib_test.c)

#define libraries targets
add_library(m2-isa lib-isa/isa.c)
add_library(m2-obj lib-obj/obj.c)
add_library(m2-ldm lib-ldm/ldm.c)
add_library(m2-mif lib-mif/mif.c)
add_library(microtar lib-microtar/src/microtar.c)
add_library(m2-sl lib-sl/sl.c)

#define libraries dep for executables
target_link_libraries(m2-ldm2mif m2-mif m2-ldm m2-isa)
target_link_libraries(m2-loader m2-ldm m2-isa)
target_link_libraries(m2-assembler m2-obj m2-isa)
target_link_libraries(m2-objread m2-obj m2-isa)
target_link_libraries(m2-sl microtar)
target_link_libraries(m2-obj m2-isa)
target_link_libraries(m2-archiver m2-obj m2-sl)

#test targets libs
target_link_libraries(objlib_test m2-obj m2-isa)
target_link_libraries(sllib_test m2-obj m2-sl)

#define where to find library headers
include_directories(lib-isa/ lib-obj/ lib-ldm/ lib-mif/ lib-microtar/src/ lib-sl/)

add_definitions(-DVERSION="v1.0 Alpha")
add_definitions(-DTARGET_ARCH_NAME="MARK_II")
