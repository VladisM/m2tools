cmake_minimum_required(VERSION 3.2.0)
project(isalib C)

add_library(isalib isa.c)
target_include_directories(isalib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
