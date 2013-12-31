cmake_minimum_required (VERSION 2.8)

project(drv_package)

add_library(drv STATIC drv_print.c)

install(TARGETS drv DESTINATION lib)
install(FILES drv_print.h DESTINATION include)
