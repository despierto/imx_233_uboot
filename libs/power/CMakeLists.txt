cmake_minimum_required (VERSION 2.8)

project(hw_power)

add_library(power STATIC hw_power.c
                            hw_power_battery.c 
                            hw_power_init.c 
                            hw_power_interrupts.c 
                            hw_power_registers.c 
                            hw_power_conversions.c 
                            hw_power_fiq.c 
                            hw_power_5v_detect.c)

install(TARGETS power DESTINATION lib)
