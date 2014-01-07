cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: core")

add_executable(xboot main.c)
target_link_libraries(xboot sysconsole)   
target_link_libraries(xboot drv)

set_target_properties(xboot PROPERTIES LINK_FLAGS "-Map ${PRJ_NAME}.map -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${PRJ_NAME}_link.lds")

install(TARGETS xboot DESTINATION bin)
