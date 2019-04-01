cmake_minimum_required(VERSION 3.2.0)
project(microtarlib C)

add_library(microtarlib src/microtar.c)
target_include_directories(microtarlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
