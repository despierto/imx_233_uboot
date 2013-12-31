cmake_minimum_required (VERSION 2.8)

project(syster_console)

add_library(sysconsole STATIC sys_console.c)

#install(TARGETS sysconsole DESTINATION lib)
#install(FILES sys_console.h DESTINATION include)
