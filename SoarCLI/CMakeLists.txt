cmake_minimum_required(VERSION 3.22)
project(soar_cli
    LANGUAGES CXX)

add_executable(soar soar_cli.cpp)
set_target_properties(soar PROPERTIES PDB_NAME soar_cli)
target_link_libraries(soar soar_lib)

install(TARGETS soar
    DESTINATION ${CMAKE_INSTALL_BINDIR})
