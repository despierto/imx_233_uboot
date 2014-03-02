cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv :net")

add_library(net STATIC 	net_ks8851.c )

install(TARGETS net DESTINATION lib)
