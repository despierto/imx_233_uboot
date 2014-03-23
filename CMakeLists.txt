cmake_minimum_required(VERSION 2.8)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cross_compile.cmake")

set(PRJ_NAME "xboot")
set(BOOT_PRJ_NAME "xinit")

project(${PRJ_NAME} C)

set(TARGET_MACH "imx-233")
set(TARGET_ARCH "armv5te")
set(TARGET_CPU "arm926ej-s")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${TARGET_CPU} -march=${TARGET_ARCH} -msoft-float" CACHE STRING "")

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-libgcc-file-name
  OUTPUT_VARIABLE TARGET_LIBGCC
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-file-name=libc.a
  OUTPUT_VARIABLE TARGET_LIBC
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

set(DIALECT "-ATT")
set(CMAKE_ASM${DIALECT}_SOURCE_FILE_EXTENSIONS "S;s;asm")
enable_language(ASM-ATT)

set(CMAKE_ASM${DIALECT}_FLAGS "-march=${TARGET_ARCH} -mcpu=${TARGET_CPU} -mlittle-endian -EL --warn --fatal-warnings")
set(CMAKE_ASM_FLAGS_DEBUG "-g")
set(CMAKE_ASM_FLAGS_RELEASE "--strip-local-absolute")

set(CMAKE_C_LINK_FLAGS "-A ${TARGET_ARCH} --gc-sections -static --stats --print-gc-sections")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> -o <TARGET> <LINK_LIBRARIES>")

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/include")

#add_definitions("-DMACH_ARM_V5")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCODEPROTENA")


add_subdirectory(sys)
add_subdirectory(drv)
add_subdirectory(net)
add_subdirectory(mach)
add_subdirectory(core)
