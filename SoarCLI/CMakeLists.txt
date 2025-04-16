cmake_minimum_required(VERSION 3.22)
project(soar_cli
    VERSION 9.6.3
    LANGUAGES CXX)

add_executable(soar soar_cli.cpp)
target_link_libraries(soar soar_lib)

install(TARGETS soar
    DESTINATION ${CMAKE_INSTALL_BINDIR})
