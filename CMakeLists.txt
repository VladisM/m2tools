cmake_minimum_required(VERSION 3.2.0)
project(m2tools C)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_STATIC_LIBS ON)

set(target_prefix "m2")
set(target_arch_name "MARK_II")
set(ver_string "v1.0 Alpha")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb -Wall")

add_definitions(-DVERSION="${ver_string}")
add_definitions(-DTARGET_ARCH_NAME="${target_arch_name}")

#define libraries targets
add_subdirectory(./isalib)
add_subdirectory(./objlib)
add_subdirectory(./microtarlib)
add_subdirectory(./sllib)

#define executable targets
add_subdirectory(./assembler)
add_subdirectory(./archiver)
add_subdirectory(./objread)
add_subdirectory(./linker)
