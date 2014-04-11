cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: sys: console")

add_library(sysconsole STATIC sys_cmgr.c
                              sys_cmd.c)

install(TARGETS sysconsole DESTINATION lib)
