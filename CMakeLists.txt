cmake_minimum_required(VERSION 3.23)
project(soar
  VERSION 9.6.5
  LANGUAGES CXX C)

set(SOAR_LIB "${PROJECT_NAME}_lib")

# Generate soarversion.h from template using project(VERSION) as single source of truth.
# The generated file goes in the build tree; SCons builds use the hand-maintained original.
configure_file(Core/shared/soarversion.h.in ${CMAKE_BINARY_DIR}/Core/shared/soarversion.h)

# Generate compile_commands.json for IDE support and static analysis tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)

# All options should be turned off by default to avoid building unnecessary
# components in downstream projects relying on cmake's FetchContent module.
# FetchContent does not allow to set options for dependencies.
option(BUILD_TESTS "Build tests" OFF)
option(CLI "Build Soar command line interface" OFF)
option(SWIG "Build all SWIG bindings" OFF)
option(SWIG_PYTHON "Build Python SWIG bindings" OFF)
option(SWIG_JAVA "Build Java SWIG bindings" OFF)
option(SWIG_JAVASCRIPT "Build JavaScript SWIG bindings" OFF)
option(SWIG_TCL "Build Tcl SWIG bindings" OFF)
option(SWIG_CSHARP "Build C# SWIG bindings" OFF)
option(JAVA_DEBUGGER "Build Java Debugger (requires SWIG_JAVA)" OFF)

if(SWIG)
  set(SWIG_PYTHON ON CACHE BOOL "Build Python SWIG bindings" FORCE)
  set(SWIG_JAVA ON CACHE BOOL "Build Java SWIG bindings" FORCE)
  set(SWIG_JAVASCRIPT ON CACHE BOOL "Build JavaScript SWIG bindings" FORCE)
  set(SWIG_TCL ON CACHE BOOL "Build Tcl SWIG bindings" FORCE)
  set(SWIG_CSHARP ON CACHE BOOL "Build C# SWIG bindings" FORCE)
endif()

# ASAN only works in debug mode.
option(ASAN "Enable AddressSanitizer" OFF)
option(SVS "SVS Enabled" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "Soar: Insallation prefix not set. Defaulting to ${CMAKE_INSTALL_PREFIX}")
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_CURRENT_SOURCE_DIR}/install")
endif()

include(GNUInstallDirs)

# Put all build outputs into the repo-root "out/" directory, matching the SCons
# layout.  This ensures DLLs, EXEs, and MODULE libraries are co-located so that
# executables can find Soar.dll at runtime without PATH manipulation, and tools
# that reference SOAR_HOME / out/ keep working regardless of build system.
set(SOAR_OUT_DIR "${CMAKE_SOURCE_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SOAR_OUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${SOAR_OUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${SOAR_OUT_DIR}")

# Copy non-binary support files into the out/ tree at configure time.
file(COPY scripts/settings.soar DESTINATION "${SOAR_OUT_DIR}")

# For Release builds, silence GTK/SWT warnings by redirecting the Debugger's
# stderr to /dev/null (or nul on Windows). Dev/Debug builds keep stderr so
# real JVM/JNI errors remain visible.
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(_soar_dbg_stderr_sh  " 2>/dev/null")
  set(_soar_dbg_stderr_bat " 2>nul")
else()
  set(_soar_dbg_stderr_sh  "")
  set(_soar_dbg_stderr_bat "")
endif()

if(WIN32)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/scripts/SoarJavaDebugger.bat" _dbg_bat)
  string(REPLACE "SoarJavaDebugger.jar %1 %2 %3 %4 %5"
                 "SoarJavaDebugger.jar %1 %2 %3 %4 %5${_soar_dbg_stderr_bat}"
                 _dbg_bat "${_dbg_bat}")
  file(WRITE "${SOAR_OUT_DIR}/SoarJavaDebugger.bat" "${_dbg_bat}")
else()
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/scripts/SoarJavaDebugger.sh" _dbg_sh)
  string(REPLACE "SoarJavaDebugger.jar\" \"\$@\" &"
                 "SoarJavaDebugger.jar\" \"\$@\"${_soar_dbg_stderr_sh} &"
                 _dbg_sh "${_dbg_sh}")
  file(WRITE "${SOAR_OUT_DIR}/SoarJavaDebugger.sh" "${_dbg_sh}")
  execute_process(COMMAND chmod +x "${SOAR_OUT_DIR}/SoarJavaDebugger.sh")
endif()

# Add your source files
set(SOURCES
  Core/ClientSML/ClientSML.cxx
  Core/ConnectionSML/ConnectionSML.cxx
  Core/ElementXML/ElementXML.cxx
  Core/KernelSML/KernelSML.cxx
  Core/SoarKernel/SoarKernel.cxx
  Core/shared/shared.cxx
  Core/CLI/CommandLineInterface.cxx
)

file(GLOB_RECURSE HEADERS
  "Core/CLI/src/*.h"
  "Core/ClientSML/src/*.h"
  "Core/ConnectionSML/src/*.h"
  "Core/ElementXML/src/*.h"
  "Core/KernelSML/src/*.h"
  "Core/SoarKernel/src/debug_code/*.h"
  "Core/SoarKernel/src/decision_process/*.h"
  "Core/SoarKernel/src/episodic_memory/*.h"
  "Core/SoarKernel/src/explanation_based_chunking/*.h"
  "Core/SoarKernel/src/explanation_memory/*.h"
  "Core/SoarKernel/src/interface/*.h"
  "Core/SoarKernel/src/output_manager/*.h"
  "Core/SoarKernel/src/parsing/*.h"
  "Core/SoarKernel/src/reinforcement_learning/*.h"
  "Core/SoarKernel/src/semantic_memory/*.h"
  "Core/SoarKernel/src/shared/*.h"
  "Core/SoarKernel/src/soar_representation/*.h"
  "Core/SoarKernel/src/visualizer/*.h"
  "Core/shared/Export.h"
  "Core/shared/build_time_date.h"
  "${CMAKE_BINARY_DIR}/Core/shared/soarversion.h"
  "Core/shared/tokenizer.h"
  "Core/shared/soar_TraceNames.h"
  "Core/shared/portability.h"
  "Core/shared/portability_posix.h"
)

# Public API headers — only these are installed / shipped in releases.
# Matches the SCons include set: ElementXML/src, ConnectionSML/src,
# ClientSML/src, and Core/shared.
file(GLOB PUBLIC_HEADERS
  "Core/ElementXML/src/*.h"
  "Core/ConnectionSML/src/*.h"
  "Core/ClientSML/src/*.h"
  "Core/shared/*.h"
  "Core/shared/*.hpp"
  "${CMAKE_BINARY_DIR}/Core/shared/soarversion.h"
)
file(GLOB PUBLIC_HEADERS_MSVC "Core/shared/msvc/*.h")

# Add additional files/dependencies if SVS is enabled
if(SVS)
  message(STATUS "Soar: Enabling SVS")
  find_package(asio REQUIRED)
  find_package(Eigen3 REQUIRED)

  # Build ccd as a static library (conan's libccd does not work for our needs)
  add_subdirectory(Core/SVS/ccd)

  list(APPEND SOURCES
    Core/SVS/SVS_cmake.cxx
  )

  file(GLOB_RECURSE SVS_HEADERS
    "Core/SVS/src/*.h"
    "Core/SVS/src/commands/*.h"
    "Core/SVS/src/filters/*.h"
  )
  list(APPEND HEADERS ${SVS_HEADERS})
else()
  message(STATUS "Soar: Disabling SVS")
  add_compile_definitions(NO_SVS)
endif()



# Add executable/library target
add_library(${SOAR_LIB} ${SOURCES})
set_target_properties(${SOAR_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${SOAR_LIB} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/CLI/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ClientSML/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ConnectionSML/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/ElementXML/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/KernelSML/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/debug_code>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/decision_process>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/episodic_memory>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/explanation_based_chunking>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/explanation_memory>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/interface>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/output_manager>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/parsing>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/reinforcement_learning>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/semantic_memory>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/shared>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/soar_representation>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SoarKernel/src/visualizer>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/Core/shared>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/shared>
  $<INSTALL_INTERFACE:include/soar>
)
target_link_libraries(${SOAR_LIB} PUBLIC SQLite::SQLite3)

if(SVS)
  target_include_directories(${SOAR_LIB} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SVS/ccd>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SVS/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SVS/src/filters>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/SVS/src/commands>
  )

  target_link_libraries(${SOAR_LIB} PUBLIC asio::asio)
  target_link_libraries(${SOAR_LIB} PUBLIC Eigen3::Eigen)
  target_link_libraries(${SOAR_LIB} PUBLIC ccd)

  # asio requires _WIN32_WINNT >= 0x0601 (Windows 7+)
  if(WIN32)
    target_compile_definitions(${SOAR_LIB} PRIVATE _WIN32_WINNT=0x0601)
  endif()
endif()


set_target_properties(${SOAR_LIB}
  PROPERTIES
  PUBLIC_HEADER "${PUBLIC_HEADERS}"
  OUTPUT_NAME "Soar")

# Unity build files (e.g. SoarKernel.cxx) exceed the default COFF section limit
# in Debug builds. /bigobj raises the limit from 2^16 to 2^32 sections.
if(MSVC)
  target_compile_options(${SOAR_LIB} PRIVATE /bigobj)
endif()

set(CXX_STANDARD_REQUIRED True)

# Set the C++ standard for this and all downstream targets
target_compile_features(${SOAR_LIB} PUBLIC
  cxx_std_17
)

# On Windows, Export.h needs a define to resolve the EXPORT macro correctly:
#   Shared builds: _USRDLL  -> __declspec(dllexport)
#   Static builds: STATIC_LINKED -> (empty)
# Without either, EXPORT resolves to __declspec(dllimport) which breaks compilation.
if(WIN32)
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${SOAR_LIB} PRIVATE _USRDLL)
  else()
    target_compile_definitions(${SOAR_LIB} PUBLIC STATIC_LINKED)
  endif()
endif()

if($<CONFIG:DEBUG>)
  target_compile_options(${SOAR_LIB} PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>: -Wall -Wpedantic>)

  target_compile_options(${SOAR_LIB} PRIVATE $<$<CXX_COMPILER_ID:MSVC>: /Wall /permissive>)

  if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if(ASAN)
      message(STATUS "Soar: Enabling AddressSanitizer")
      add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
      add_link_options(-fsanitize=address)
    endif()

    find_program(CLANG_TIDY_EXE NAMES clang-tidy)

    if(CLANG_TIDY_EXE)
      set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -checks=-*,readability-*)
    else()
      message(WARNING "clang-tidy not found!")
    endif()
  else()
    message(WARNING "ASAN Debug flags for Windows not set up.")
  endif()
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/soarConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

# This only sets up the install structure, but no installation happens yet.
install(TARGETS ${SOAR_LIB}
  EXPORT SoarTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${PUBLIC_HEADERS_MSVC} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/msvc)

install(EXPORT SoarTargets
  FILE SoarTargets.cmake
  NAMESPACE soar::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soar
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/soarConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/soar"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/soarConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/soarConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/soar
)

export(EXPORT SoarTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/SoarTargets.cmake"
)

add_library(soar::soar_lib ALIAS ${SOAR_LIB})

if(CLI)
  message(STATUS "Soar: Building CLI")
  add_subdirectory(SoarCLI)
endif()

if(SWIG OR SWIG_PYTHON OR SWIG_JAVA OR SWIG_JAVASCRIPT OR SWIG_TCL OR SWIG_CSHARP)
  message(STATUS "Soar: Building SWIG language bindings")
  add_subdirectory(Core/ClientSMLSWIG)
endif()

if(JAVA_DEBUGGER)
  if(NOT SWIG_JAVA)
    message(FATAL_ERROR "JAVA_DEBUGGER requires SWIG_JAVA=ON (for sml.jar)")
  endif()
  message(STATUS "Soar: Building Java Debugger")
  add_subdirectory(Java)
endif()

# TclSoarLib — Tcl interpreter embedded in Soar (loaded at runtime).
# Separate from SWIG_TCL (Tcl_sml_ClientInterface), which is the SML binding.
add_subdirectory(Tcl)

if(BUILD_TESTS)
  message(STATUS "Soar: Building tests")
  include(CTest)

  if(NOT DEFINED BUILD_PERFORMANCE_TESTS OR BUILD_PERFORMANCE_TESTS)
    add_subdirectory(PerformanceTests)
  endif()

  add_subdirectory(UnitTests)
endif()

# Install support files (Debugger launcher comes from SOAR_OUT_DIR so it reflects
# the per-build-type stderr-redirect decision above).
install(FILES scripts/settings.soar DESTINATION .)
if(WIN32)
  install(FILES "${SOAR_OUT_DIR}/SoarJavaDebugger.bat" DESTINATION .)
else()
  install(PROGRAMS "${SOAR_OUT_DIR}/SoarJavaDebugger.sh" DESTINATION .)
endif()

# Set CPack variables
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Soar is a cognitive architecture for developing systems that exhibit intelligent behavior.")
set(CPACK_PACKAGE_CONTACT "soar-admin@umich.edu")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VENDOR "University of Michigan Soar Group")
list(APPEND CPACK_GENERATOR "TGZ")

if(WIN32)
  list(APPEND CPACK_GENERATOR "ZIP")
endif()

if(UNIX AND NOT APPLE)
  list(APPEND CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

# Include CPack module - MUST be included last!
include(CPack)
