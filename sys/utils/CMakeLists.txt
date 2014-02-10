cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: sys: utils")

add_library(sysutils STATIC sys_utils.c)

install(TARGETS sysutils DESTINATION lib)
