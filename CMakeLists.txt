cmake_minimum_required(VERSION 3.2.0)
project(m2tools C)
set(CMAKE_BUILD_TYPE Debug)

#define executable targets
add_executable(m2-ldm2mif   ldm2mif/main.c)
add_executable(m2-loader    loader/main.c loader/sender.c)
add_executable(m2-assembler assembler/file_gen.c assembler/main.c assembler/pass1.c assembler/pass2.c assembler/symbol_table.c assembler/tokenizer.c assembler/util.c)
add_executable(m2-objread   objread/main.c)

#define libraries targets
add_library(m2-isa STATIC lib-isa/isa.c)
add_library(m2-obj STATIC lib-obj/obj.c)
add_library(m2-ldm STATIC lib-ldm/ldm.c)
add_library(m2-mif STATIC lib-mif/mif.c)

#define libraries dep for executables
target_link_libraries(m2-ldm2mif   m2-mif m2-ldm        m2-isa)
target_link_libraries(m2-loader           m2-ldm        m2-isa)
target_link_libraries(m2-assembler               m2-obj m2-isa)
target_link_libraries(m2-objread                 m2-obj m2-isa)

#define where to find library headers
include_directories(lib-isa/ lib-obj/ lib-ldm/ lib-mif/)

add_definitions(-DVERSION="v1.0 Alpha")
