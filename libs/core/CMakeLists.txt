cmake_minimum_required (VERSION 2.8)

project(hw_core)

add_library(core STATIC hw_core_asm_interrupt.S)

install(TARGETS core DESTINATION lib)
