cmake_minimum_required(VERSION 3.23)
project(soar
  VERSION 9.6.3
  LANGUAGES CXX)

find_package(SQLite3 REQUIRED)

option(SYSTEM_INSTALL "Install to system directories" OFF)
option(BUILD_TESTS "Build tests" OFF)

# ASAN only works in debug mode.
option(ASAN "Enable AddressSanitizer" OFF)
option(SVS "SVS Enabled" OFF)

if(SYSTEM_INSTALL)
  include(GNUInstallDirs)
else()
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Installation Directory" FORCE)
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
  "Core/shared/*.h"
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
  PUBLIC_HEADER "${HEADERS}")

set(CXX_STANDARD_REQUIRED True)

# Set the C++ standard for this and all downstream targets
target_compile_features(${PROJECT_NAME}_lib PUBLIC
  cxx_std_17
)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  target_compile_options(${PROJECT_NAME}_lib PRIVATE -Wall -Wpedantic)

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
    message(WARNING "Debug flags for windows not set up.")
  endif()
endif()

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
  INSTALL_DESTINATION "lib/cmake/soar"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/soarConfig.cmake
  DESTINATION lib/cmake/soar
)

export(EXPORT SoarTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/SoarTargets.cmake"
)

add_library(soar::soar ALIAS soar_lib)

add_subdirectory(SoarCLI)

if(BUILD_TESTS)
  include(CTest)
  add_subdirectory(PerformanceTests)
  add_subdirectory(UnitTests)
endif()
