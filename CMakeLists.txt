cmake_minimum_required(VERSION 3.23)
project(soar
  VERSION 9.6.3
  LANGUAGES CXX)

find_package(SQLite3 REQUIRED)

# All options should be turned off by default to avoid building unnecessary
# components in downstream projects relying on cmake's FetchContent module.
# FetchContent does not allow to set options for dependencies.
option(BUILD_TESTS "Build tests" OFF)
option(CLI "Build Soar command line interface" OFF)

# ASAN only works in debug mode.
option(ASAN "Enable AddressSanitizer" OFF)
option(SVS "SVS Enabled" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "Soar: Insallation prefix not set. Defaulting to ${CMAKE_INSTALL_PREFIX}")
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_CURRENT_SOURCE_DIR}/install")
endif()

include(GNUInstallDirs)

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
  "Core/shared/soarversion.h"
  "Core/shared/tokenizer.h"
  "Core/shared/soar_TraceNames.h"
  "Core/shared/portability.h"
  "Core/shared/portability_posix.h"
)

if(NOT SVS)
  message(STATUS "Soar: Disabling SVS")
  add_compile_definitions(NO_SVS)
endif()

# Add executable/library target
add_library(${PROJECT_NAME}_lib ${SOURCES})
target_include_directories(${PROJECT_NAME}_lib PUBLIC
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
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/shared>
  $<INSTALL_INTERFACE:include/soar>
)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC SQLite::SQLite3)

set_target_properties(${PROJECT_NAME}_lib
  PROPERTIES
  PUBLIC_HEADER "${HEADERS}"
  OUTPUT_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

set(CXX_STANDARD_REQUIRED True)

# Set the C++ standard for this and all downstream targets
target_compile_features(${PROJECT_NAME}_lib PUBLIC
  cxx_std_17
)

if($<CONFIG:DEBUG>)
  target_compile_options(${PROJECT_NAME}_lib PRIVATE $<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>: -Wall -Wpedantic>)

  target_compile_options(${PROJECT_NAME}_lib PRIVATE $<$<CXX_COMPILER_ID:MSVC>: /Wall /permissive>)

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
install(TARGETS ${PROJECT_NAME}_lib
  EXPORT SoarTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/soar
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/soar
)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/soar)

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

add_library(soar::soar_lib ALIAS ${PROJECT_NAME}_lib)

if(CLI)
  message(STATUS "Soar: Building CLI")
  add_subdirectory(SoarCLI)
endif()

if(BUILD_TESTS)
  message(STATUS "Soar: Building tests")
  include(CTest)
  add_subdirectory(PerformanceTests)
  add_subdirectory(UnitTests)
endif()

# Set CPack variables
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Soar is a cognitive architecture for developing systems that exhibit intelligent behavior.")
set(CPACK_PACKAGE_CONTACT "soar-admin@umich.edu")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VENDOR "University of Michigan Soar Group")
list(APPEND CPACK_GENERATOR "TGZ")

if(UNIX AND NOT APPLE)
  list(APPEND CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

# Include CPack module - MUST be included last!
include(CPack)
