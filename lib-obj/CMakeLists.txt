cmake_minimum_required(VERSION 3.2.0)
project(objlib C)

add_library(objlib obj.c)
target_include_directories(objlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(objlib PUBLIC isalib)
