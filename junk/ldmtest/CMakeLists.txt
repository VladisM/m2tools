cmake_minimum_required(VERSION 3.2.0)
project(ldmtest C)

set(sources ldmtest.c)

add_executable(ldmtest ${sources})

target_link_libraries(ldmtest ldmlib)
