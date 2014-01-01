cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: digctl")

project(hw_digctl)

add_library(digctl STATIC   hw_digctl.c)

install(TARGETS digctl DESTINATION lib)
