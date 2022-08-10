cmake_minimum_required(VERSION 3.13.0)
project(m2tools C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g -O0)
    IF(UNIX)
        add_link_options(-rdynamic)
        add_definitions(-DENABLE_CALL_STACK_DUMP_AT_ERROR)
    ENDIF()
ELSE()
    add_compile_options(-O2)
ENDIF()

set(CMAKE_C_STANDARD 99)

set(ver_string "v1.0")

add_definitions(-DVERSION="${ver_string}")

add_compile_options(-Wall -Wextra)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/utillib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/cwalk)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/platformlib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/filelib)

##############################
# source files

set(assembler_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/assembler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/preprocessor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/preprocessor_symbol_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/section_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/symbol_table.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/pass1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/pass2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/filegen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/pass_item.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assembler/verbose.c
)

set(archiver_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/archiver/archiver.c
)

set(objread_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/objread/objread.c
)

set(linker_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linker/linker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linker/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linker/ldparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linker/cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/linker/link.c
)

set(ldmdump_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldmdump/ldmdump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldmdump/mif_backend.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldmdump/ihex_backend.c
)

##############################
# targets

add_executable(${platformlib_target_prefix}-assembler ${assembler_sources})
target_link_libraries(${platformlib_target_prefix}-assembler PRIVATE utillib-core utillib-utils utillib-cli filelib platformlib)
target_compile_definitions(${platformlib_target_prefix}-assembler PRIVATE -DPROG_NAME="${platformlib_target_prefix}-assembler")

add_executable(${platformlib_target_prefix}-archiver ${archiver_sources})
target_link_libraries(${platformlib_target_prefix}-archiver PRIVATE utillib-core utillib-cli filelib)
target_compile_definitions(${platformlib_target_prefix}-archiver PRIVATE -DPROG_NAME="${platformlib_target_prefix}-archiver")

add_executable(${platformlib_target_prefix}-objread ${objread_sources})
target_link_libraries(${platformlib_target_prefix}-objread PRIVATE utillib-core utillib-cli filelib platformlib)
target_compile_definitions(${platformlib_target_prefix}-objread PRIVATE -DPROG_NAME="${platformlib_target_prefix}-objread")

add_executable(${platformlib_target_prefix}-linker ${linker_sources})
target_link_libraries(${platformlib_target_prefix}-linker PRIVATE utillib-core utillib-utils utillib-cli filelib platformlib)
target_compile_definitions(${platformlib_target_prefix}-linker PRIVATE -DPROG_NAME="${platformlib_target_prefix}-linker")

add_executable(${platformlib_target_prefix}-ldmdump ${ldmdump_sources})
target_link_libraries(${platformlib_target_prefix}-ldmdump PRIVATE utillib-core utillib-cli filelib utillib-files)
target_compile_definitions(${platformlib_target_prefix}-ldmdump PRIVATE -DPROG_NAME="${platformlib_target_prefix}-ldmdump")
