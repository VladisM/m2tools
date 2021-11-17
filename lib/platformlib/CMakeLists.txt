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

if(NOT TARGET_ARCH)
    message(FATAL_ERROR "TARGET_ARCH is not set! Please specify backend with TARGET_ARCH variable!")
endif()

if(TARGET_ARCH MATCHES "example")
    set(platformlib_target_prefix "example" PARENT_SCOPE)
    set(platformlib_target_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/targets/example/example.c
    )
    set(platformlib_target_includes
        ${CMAKE_CURRENT_SOURCE_DIR}/targets/example/include/
    )
else()
    message(FATAL_ERROR "Specified target wasn't found in platformlib!")
endif()

# end of target specific sources
# ------------------------------------------------------------------------------

set(platformlib_common_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platformlib_common.c
)

set(platformlib_common_includes
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_library(platformlib
    ${platformlib_target_sources}
    ${platformlib_common_sources}
)

target_include_directories(platformlib PUBLIC
    ${platformlib_target_includes}
    ${platformlib_common_includes}
)

target_link_libraries(platformlib PRIVATE
    utillib-core
    utillib-utils
)
