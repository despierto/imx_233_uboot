cmake_minimum_required (VERSION 2.8)
project(libtest_project1)
#add_library(test1 SHARED test1.c)
add_library(test1 STATIC test1.c)

install(TARGETS test1 DESTINATION lib)
install(FILES test1.h DESTINATION include)
