cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: drv")

add_library(drv STATIC 	net/drv_ks8851.c
                        spi/drv_spi.c
                        drv_utils.c	
                        drv_eth.c ) 

install(TARGETS drv DESTINATION lib)

