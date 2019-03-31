cmake_minimum_required(VERSION 3.2.0)
project(sllib C)

add_library(sllib sl.c)
target_include_directories(sllib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sllib PRIVATE microtarlib)
target_link_libraries(sllib PUBLIC objlib)
