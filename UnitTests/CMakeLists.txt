cmake_minimum_required(VERSION 3.22)
project(test_soar
    LANGUAGES CXX)

file(GLOB TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/wma/*.cpp
)

file(GLOB TEST_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/wma/*.hpp
)

add_executable(${PROJECT_NAME} TestHarness/testMain.cpp ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} soar_lib)
target_compile_features(${PROJECT_NAME} PUBLIC
    cxx_std_17
)

add_executable(test_external_lib TestExternalLibraryLib.cpp)
target_link_libraries(test_external_lib soar_lib)
target_compile_features(test_external_lib PUBLIC
    cxx_std_17
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/wma
)

file(COPY ${CMAKE_SOURCE_DIR}/UnitTests/SoarTestAgents DESTINATION ${CMAKE_BINARY_DIR}/UnitTests)

# TODO: re-enable SVS tests once we implement the CMake build for it
# TODO: re-enable testLoadLibrary once we implement the CMake build for it
# TESTS FROM CI:
if(BUILD_TESTING)
    add_test(NAME test_ci COMMAND test_soar -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton -E SvsTests)

    if(SVS)
        add_test(NAME test_svs COMMAND test_soar -c SvsTests)
    endif()
endif()
