cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: sys: heap")

add_library(sysheap STATIC sys_heap.c)

install(TARGETS sysheap DESTINATION lib)
