cmake_minimum_required(VERSION 3.2.0)
project(miflib C)

add_library(miflib mif.c)
target_include_directories(miflib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
