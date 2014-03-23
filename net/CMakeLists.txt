cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: net")

add_library(net STATIC  net.c 
                        net_arp.c )

install(TARGETS net DESTINATION lib)

