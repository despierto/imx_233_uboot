cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv :spi")

add_library(spi STATIC 	spi.c )

install(TARGETS spi DESTINATION lib)
