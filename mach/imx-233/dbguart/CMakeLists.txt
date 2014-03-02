cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: dbguart")

add_library(dbguart STATIC dbguart.c)

install(TARGETS dbguart DESTINATION lib)
