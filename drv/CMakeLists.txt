cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv")

add_library(drv STATIC drv_print.c)

install(TARGETS drv DESTINATION lib)
