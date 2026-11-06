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

add_library(TestExternalLibraryLib SHARED TestExternalLibraryLib.cpp)
target_link_libraries(TestExternalLibraryLib soar_lib)
target_compile_features(TestExternalLibraryLib PUBLIC
    cxx_std_17
)
if(WIN32)
    set_target_properties(TestExternalLibraryLib PROPERTIES PREFIX "")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarHelpers
    ${CMAKE_CURRENT_SOURCE_DIR}/TestHarness
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests
    ${CMAKE_CURRENT_SOURCE_DIR}/SoarUnitTests/wma
)

# Copy test agents to the out/ tree so the test exe can find them at runtime.
# SoarHelper::ResourceDirectory is "./SoarUnitTests/" — UnitTests.exe runs with
# CWD=${SOAR_OUT_DIR} and looks up "./SoarUnitTests/<file>.soar" directly, so
# the *contents* of SoarTestAgents/ must land in SoarUnitTests/ (mirrors the
# SCons layout: InstallDir('$OUT_DIR/SoarUnitTests/', 'SoarTestAgents')).
file(GLOB _soar_test_agent_entries
     LIST_DIRECTORIES TRUE
     "${CMAKE_SOURCE_DIR}/UnitTests/SoarTestAgents/*")
file(COPY ${_soar_test_agent_entries}
     DESTINATION ${SOAR_OUT_DIR}/SoarUnitTests)

install(TARGETS ${PROJECT_NAME} TestExternalLibraryLib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
# Install contents of SoarTestAgents/ as SoarUnitTests/ in the install tree so
# the release zip's UnitTests.exe (with CWD next to it) can find them.
install(DIRECTORY ${CMAKE_SOURCE_DIR}/UnitTests/SoarTestAgents/
        DESTINATION SoarUnitTests)
# TESTS FROM CI:
if(BUILD_TESTING)
    # Add CI tests only if TEST_CI_ENABLED is enabled (default ON if not specified)
    if(NOT DEFINED TEST_CI_ENABLED OR TEST_CI_ENABLED)
        add_test(NAME test_ci
            COMMAND ${PROJECT_NAME} -e PRIMS_Sanity1 -e PRIMS_Sanity2 -f testLoadLibrary -f testSmemArithmetic -f testHamilton -f Demo_Arithmetic -E SvsTests
            WORKING_DIRECTORY "${SOAR_OUT_DIR}"
        )

        # With CMAKE_RUNTIME_OUTPUT_DIRECTORY set, Soar.dll is in the same dir
        # as UnitTests.exe, so no PATH manipulation is needed on Windows.
    endif()

    if(SVS)
        add_test(NAME test_svs
            COMMAND ${PROJECT_NAME} -c SvsTests
            WORKING_DIRECTORY "${SOAR_OUT_DIR}"
        )
    endif()

    # Python SML Interface Test
    if(SWIG_PYTHON)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        if(WIN32)
            set(_PY_PATH_SEP "\\;")
        else()
            set(_PY_PATH_SEP ":")
        endif()
        set(PYTHON_BUILD_DIR "${CMAKE_BINARY_DIR}/Core/ClientSMLSWIG/python")
        set(PYTHON_LIBRARY_DIR "${CMAKE_BINARY_DIR}/Core/ClientSMLSWIG")

        add_test(NAME test_python_sml
            COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/test_python_sml.py"
            WORKING_DIRECTORY "${SOAR_OUT_DIR}"
        )

        set_tests_properties(test_python_sml PROPERTIES
            ENVIRONMENT "PYTHONPATH=${SOAR_OUT_DIR}${_PY_PATH_SEP}${PYTHON_BUILD_DIR}${_PY_PATH_SEP}${PYTHON_LIBRARY_DIR};SOAR_BUILD_DIR=${CMAKE_BINARY_DIR};PYTHONUTF8=1"
        )
    endif()

    # Java SML Interface Test
    if(SWIG_JAVA)
        find_package(Java REQUIRED COMPONENTS Runtime)
        if(TARGET build_smljava)
            add_test(NAME test_java_sml
                COMMAND ${Java_JAVA_EXECUTABLE} -Djava.library.path=${SOAR_OUT_DIR} -jar "${SOAR_OUT_DIR}/java/soar-smljava.jar"
                WORKING_DIRECTORY "${SOAR_OUT_DIR}"
            )
        else()
            add_test(NAME test_java_sml
                COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/test_java_sml.sh"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
            set_tests_properties(test_java_sml PROPERTIES
                ENVIRONMENT "SOAR_BUILD_DIR=${CMAKE_BINARY_DIR}"
            )
        endif()
    endif()

    # Tcl SML Interface Test
    if(SWIG_TCL)
        find_package(TCL QUIET)
        if(TCL_TCLSH)
            add_test(NAME test_tcl_sml
                COMMAND ${TCL_TCLSH} "${SOAR_OUT_DIR}/TestTclSML.tcl"
                WORKING_DIRECTORY "${SOAR_OUT_DIR}"
            )
        endif()
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
