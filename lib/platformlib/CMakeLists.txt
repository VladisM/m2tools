cmake_minimum_required(VERSION 3.13.0)
project(platformlib C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g -O0)
endif()

set(CMAKE_C_STANDARD 99)
set(BUILD_STATIC_LIBS ON)

add_compile_options(-Wall -Wextra)

# ------------------------------------------------------------------------------
# target specific sources (to add target, add elseif here)

message(FATAL_ERROR "Target not found in platformlib!")

# end of target specific sources
# ------------------------------------------------------------------------------

add_library(platformlib
    ${platformlib_target_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platformlib_common.c
)

target_include_directories(platformlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

target_compile_definitions(platformlib PUBLIC -D${platformlib_target_name})

target_link_libraries(platformlib PRIVATE utillib-core)
target_link_libraries(platformlib PRIVATE utillib-utils)
