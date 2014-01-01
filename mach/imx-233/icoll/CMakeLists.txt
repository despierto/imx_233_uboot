cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: icall")

add_library(icall STATIC    hw_icoll_set.c
                            hw_icoll_init.c) 

install(TARGETS icall DESTINATION lib)
