cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: pinmux")

add_library(pinmux STATIC pinmux.c)

install(TARGETS pinmux DESTINATION lib)
