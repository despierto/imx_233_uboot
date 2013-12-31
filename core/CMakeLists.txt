cmake_minimum_required (VERSION 2.8)

add_executable(xboot main.c)
target_link_libraries(xboot sysconsole)   
target_link_libraries(xboot drv)

install(TARGETS xboot DESTINATION bin)
