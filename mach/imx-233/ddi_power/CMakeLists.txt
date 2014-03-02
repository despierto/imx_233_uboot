cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: ddi_power")

add_library(ddi_power STATIC    ddi_power_init.c
                                ddi_power.c
                                ddi_power_handoff.c
                                ddi_power_battery.c
                                ddi_power_5v_detect.c) 

install(TARGETS ddi_power DESTINATION lib)
