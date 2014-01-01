cmake_minimum_required (VERSION 2.8)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cross_compile.cmake")

set(PRJ_NAME xboot)
set(BOOT_PRJ_NAME prep)
set(TARGET_MACH imx-233)
set(TARGET_ARCH armv5te)
set(TARGET_CPU arm926ej-s)

project(${PRJ_NAME} C)

set(CMAKE_ASM_COMPILER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}-as")

enable_language(ASM)

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/include")

#add_definitions("-DMACH_ARM_V5")

set(CMAKE_C_FLAGS "-march=${TARGET_ARCH} -mcpu=${TARGET_CPU}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=iso9899:1999 -pedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funsigned-char")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-variadic-macros")

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")

# SET(CMAKE_ASM${ASM_DIALECT}_COMPILE_OBJECT
#      "<CMAKE_ASM${ASM_DIALECT}_COMPILER> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_C_LINK_FLAGS "-A ${TARGET_ARCH} -nostdlib --gc-sections -static -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${PRJ_NAME}_link.lds -Map ${PRJ_NAME}.map --stats --print-gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L${CROSS_COMPILER_PATH}/lib/gcc/${CROSS_COMPILER_PREFIX}/4.6.3")

set(CMAKE_LINKER "${CROSS_COMPILER_PATH}/bin/${CROSS_COMPILER_PREFIX}-ld")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_ASM_FLAGS "-march=${TARGET_ARCH} -mcpu=${TARGET_CPU} -mlittle-endian -EL --warn --fatal-warnings")
set(CMAKE_ASM_FLAGS_DEBUG "-g")
set(CMAKE_ASM_FLAGS_RELEASE "--strip-local-absolute")


add_subdirectory(sys)
add_subdirectory(drv)
add_subdirectory(mach)
add_subdirectory(core)

