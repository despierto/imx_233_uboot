cmake_minimum_required (VERSION 2.8)

message(STATUS "xboot: core")

add_executable(xboot main.c main.S)
target_link_libraries(xboot sysconsole)   
target_link_libraries(xboot net)
target_link_libraries(xboot drv)
target_link_libraries(xboot pinmux)
target_link_libraries(xboot clock)
target_link_libraries(xboot sysutils)
target_link_libraries(xboot dbguart)
target_link_libraries(xboot syslist)
target_link_libraries(xboot sysheap)

target_link_libraries(xboot ${TARGET_LIBC} ${TARGET_LIBGCC}) # order does matter
set_target_properties(xboot PROPERTIES LINK_FLAGS "-Map ${PRJ_NAME}.map -T ${CMAKE_SOURCE_DIR}/mach/${TARGET_MACH}/boot/${PRJ_NAME}_link.lds")

install(TARGETS xboot DESTINATION bin)
