cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: lradc")

add_library(lradc STATIC    hw_lradc.c
                            hw_lradc_setup.c
                            hw_lradc_init.c  
                            hw_lradc_ladder.c
                            hw_lradc_touch_screen.c
                            hw_lradc_temperature_sensor.c)

install(TARGETS lradc DESTINATION lib)
