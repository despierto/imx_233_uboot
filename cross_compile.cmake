cmake_policy(VERSION 2.8)

# targeting an ARM embedded system, bare-metal
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
# set(CMAKE_SYSTEM_VERSION 1.1)

# find the target environment prefix
execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-sysroot
  OUTPUT_VARIABLE CMAKE_FIND_ROOT_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# find the canonical path to the directory
get_filename_component(CMAKE_FIND_ROOT_PATH
  "${CMAKE_FIND_ROOT_PATH}" REALPATH
  )

set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} CACHE FILEPATH "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS
  "-std=iso9899:1999 -Wstrict-prototypes -pedantic"
  " -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align"
  # "-Wconversion -Wmissing-prototypes -Werror -Wall"
  "-ffunction-sections -fdata-sections"
  "-funsigned-char -fno-builtin -nostdlib"
  "-Wno-variadic-macros"
  )

string(REGEX REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "")

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb" CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG" CACHE STRING "")

set(BUILD_SHARED_LIBS OFF)
