cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: mem")

add_library(mem STATIC    hw_mem.c)

install(TARGETS mem DESTINATION lib)
