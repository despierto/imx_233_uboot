cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv")

project(drv_package)

add_library(drv STATIC drv_print.c)

install(TARGETS drv DESTINATION lib)
