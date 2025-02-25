cmake_minimum_required(VERSION 3.22)
project(performance_tests
    LANGUAGES CXX)

# The executable is also called PerformanceTests by scons.
set(EXEC_NAME PerformanceTests)
add_executable(${EXEC_NAME} PerformanceTests.cpp)
target_link_libraries(${EXEC_NAME} soar_static)
target_compile_features(${EXEC_NAME} PUBLIC
    cxx_std_17
)

# install(DIRECTORY TestAgents DESTINATION ${CMAKE_INSTALL_PREFIX}/SoarPerformanceTests/)
install(DIRECTORY TestAgents/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/PerformanceTests/)
install(FILES do_performance_test.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/PerformanceTests/)

install(TARGETS ${EXEC_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/PerformanceTests/)
