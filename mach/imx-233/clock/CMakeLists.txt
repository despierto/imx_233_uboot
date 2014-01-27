cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: clock")

add_library(clock STATIC clock.c)

install(TARGETS clock DESTINATION lib)
