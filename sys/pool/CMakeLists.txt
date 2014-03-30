cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: sys: pool")

add_library(syspool STATIC sys_pool.c)

install(TARGETS syspool DESTINATION lib)
