cmake_minimum_required (VERSION 2.8)

project(sys_console)

add_library(console STATIC console.c)

install(TARGETS console DESTINATION lib)
install(FILES console.h DESTINATION include)
