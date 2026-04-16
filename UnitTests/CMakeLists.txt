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
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "UnitTests")
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

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/UnitTests/SoarTestAgents DESTINATION .)
# TODO: re-enable testLoadLibrary once we implement the CMake build for it
# TESTS FROM CI:
if(BUILD_TESTING)
    # Add CI tests only if TEST_CI_ENABLED is enabled (default ON if not specified)
    if(NOT DEFINED TEST_CI_ENABLED OR TEST_CI_ENABLED)
        add_test(NAME test_ci COMMAND ${PROJECT_NAME} -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton -f Demo_ToH_Recursive -f Demo_Arithmetic -E SvsTests)

        # On Windows shared builds, the test executable needs Soar.dll on PATH.
        if(WIN32 AND BUILD_SHARED_LIBS)
            set_tests_properties(test_ci PROPERTIES
                ENVIRONMENT "PATH=$<TARGET_FILE_DIR:soar_lib>;$ENV{PATH}"
            )
        endif()
    endif()

    if(SVS)
        add_test(NAME test_svs COMMAND ${PROJECT_NAME} -c SvsTests)
    endif()

    # Python SML Interface Test
    if(SWIG_PYTHON)
        # Define path to Python build artifacts
        set(PYTHON_BUILD_DIR "${CMAKE_BINARY_DIR}/Core/ClientSMLSWIG/python")
        set(PYTHON_LIBRARY_DIR "${CMAKE_BINARY_DIR}/Core/ClientSMLSWIG")

        add_test(NAME test_python_sml
            COMMAND ${Python3_EXECUTABLE} test_python_sml.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/UnitTests
        )

        # Set environment to use build directory instead of install directory
        set_tests_properties(test_python_sml PROPERTIES
            ENVIRONMENT "PYTHONPATH=${PYTHON_BUILD_DIR}:${PYTHON_LIBRARY_DIR};SOAR_BUILD_DIR=${CMAKE_BINARY_DIR}"
        )
    endif()

    # Java SML Interface Test
    if(SWIG_JAVA)
        # Use shell script approach as it's more reliable for library loading
        # The shell script handles the Java library path correctly
        add_test(NAME test_java_sml
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_java_sml.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        # Set environment to use correct build directory
        set_tests_properties(test_java_sml PROPERTIES
            ENVIRONMENT "SOAR_BUILD_DIR=${CMAKE_BINARY_DIR}"
        )
    endif()

    # JavaScript SML Interface Test
    if(SWIG_JAVASCRIPT)
        add_test(NAME test_javascript_sml
            COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test_javascript_sml.sh
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        set_tests_properties(test_javascript_sml PROPERTIES
            ENVIRONMENT "SOAR_BUILD_DIR=${CMAKE_BINARY_DIR}"
        )
    endif()
endif()
