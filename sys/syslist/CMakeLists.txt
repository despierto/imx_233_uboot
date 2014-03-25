cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: sys: list")

add_library(syslist STATIC sys_list.c)

install(TARGETS syslist DESTINATION lib)
