cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: core")


# SET(CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT
#      "<CMAKE_ASM${ASM_DIALECT}_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_C_LINK_FLAGS "-A ${TARGET_ARCH} -nostdlib --gc-sections -static -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${PRJ_NAME}_link.lds -Map ${PRJ_NAME}.map --stats --print-gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L${CROSS_COMPILER_PATH}/lib/gcc/${CROSS_COMPILER_PREFIX}/4.6.3")

set(CMAKE_LINKER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}-ld")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_ASM_FLAGS "-march=${TARGET_ARCH} -mcpu=${TARGET_CPU} -mlittle-endian -EL --warn --fatal-warnings")


add_executable(xboot main.c)
target_link_libraries(xboot sysconsole)   
target_link_libraries(xboot drv)

install(TARGETS xboot DESTINATION bin)
