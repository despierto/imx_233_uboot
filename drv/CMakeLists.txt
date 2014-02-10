cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv")

add_library(drv STATIC 	drv_utils.c	
                        drv_eth.c )

install(TARGETS drv DESTINATION lib)

add_subdirectory(spi)
add_subdirectory(net)

