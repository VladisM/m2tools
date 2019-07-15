cmake_minimum_required(VERSION 3.2.0)
project(ldmlib C)

add_library(ldmlib ldm.c)
target_include_directories(ldmlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ldmlib PUBLIC isalib)
