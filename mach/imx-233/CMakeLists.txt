cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: add subdirectories")

add_subdirectory(dbguart)
add_subdirectory(pinmux)
add_subdirectory(clock)
add_subdirectory(mem)
add_subdirectory(power)
add_subdirectory(lradc)
add_subdirectory(icoll)
add_subdirectory(digctl)
add_subdirectory(ddi_power)
add_subdirectory(hw_init)
