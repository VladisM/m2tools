cmake_minimum_required(VERSION 3.13.0)
project(filelib C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g -O0)
endif()

set(CMAKE_C_STANDARD 99)
set(BUILD_STATIC_LIBS ON)

add_compile_options(-Wall -Wextra)

set(filelib_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loading_loop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/obj.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mif.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/struct_check.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/writing_loop.c
)

add_library(filelib ${filelib_sources})

target_include_directories(filelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_link_libraries(filelib PUBLIC utillib-core platformlib)
target_link_libraries(filelib PRIVATE utillib-utils cwalk)

if(BUILD_TESTS)
    set(filelib_test_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/test/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/ldm_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/obj_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/sl_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/test/mif_test.c
    )

    add_executable(test-${platformlib_target_prefix}-filelib ${filelib_test_sources})
    target_link_libraries(test-${platformlib_target_prefix}-filelib PRIVATE utillib-cli utillib-core filelib)
    target_compile_definitions(test-${platformlib_target_prefix}-filelib PRIVATE -DPROG_NAME="test-${platformlib_target_prefix}-filelib")
endif()
