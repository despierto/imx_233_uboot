cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: mach: imx-233: hw_init")

# SET(CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT
#      "<CMAKE_ASM${ASM_DIALECT}_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_C_LINK_FLAGS "-A ${TARGET_ARCH} -nostdlib --gc-sections -static -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${BOOT_PRJ_NAME}_link.lds -Map ${BOOT_PRJ_NAME}.map --stats --print-gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L${CROSS_COMPILER_PATH}/lib/gcc/${CROSS_COMPILER_PREFIX}/4.5.1")

set(CMAKE_LINKER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}-ld")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_ASM_FLAGS "-march=${TARGET_ARCH} -mcpu=${TARGET_CPU} -mlittle-endian -EL --warn --fatal-warnings")

add_executable(hw_init hw_init.c hw_init.S)
target_link_libraries(hw_init ddi_power)
target_link_libraries(hw_init power)
target_link_libraries(hw_init lradc)
target_link_libraries(hw_init icall)
target_link_libraries(hw_init digctl)
target_link_libraries(hw_init drv)
target_link_libraries(hw_init sysutils)


target_link_libraries(hw_init gcc) # should be at the very end


install(TARGETS hw_init DESTINATION bin)



