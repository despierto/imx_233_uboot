cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: hw_init")

add_executable(hw_init hw_init.c hw_init.S)
target_link_libraries(hw_init mem)
target_link_libraries(hw_init ddi_power)
target_link_libraries(hw_init power)
target_link_libraries(hw_init lradc)
target_link_libraries(hw_init icall)
target_link_libraries(hw_init digctl)
target_link_libraries(hw_init sysutils)
target_link_libraries(hw_init drv)

target_link_libraries(hw_init ${TARGET_LIBC} ${TARGET_LIBGCC}) # order does matter
set_target_properties(hw_init PROPERTIES LINK_FLAGS "-Map ${BOOT_PRJ_NAME}.map -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${BOOT_PRJ_NAME}_link.lds")

install(TARGETS hw_init DESTINATION bin)
