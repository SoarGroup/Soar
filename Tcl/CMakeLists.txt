# TclSoarLib — embeds a Tcl interpreter inside Soar, loaded at runtime
# via the Register_Library / sml_InitLibrary interface.
#
# This is separate from the SWIG Tcl bindings (Tcl_sml_ClientInterface),
# which expose the SML API to Tcl scripts. TclSoarLib does the reverse:
# it lets Soar source Tcl scripts and use Tcl as an extension language.

option(TCLSOARLIB "Build TclSoarLib (Tcl interpreter embedded in Soar)" OFF)

if(NOT TCLSOARLIB)
  return()
endif()

message(STATUS "Soar: Building TclSoarLib")

# Find only Tcl (not Tk) — FindTCL's REQUIRED checks for both TCL+TK
find_package(TCL QUIET)
if(NOT TCL_FOUND)
    message(FATAL_ERROR "Could not find Tcl development files (TCL_LIBRARY and TCL_INCLUDE_PATH). "
        "Set CMAKE_PREFIX_PATH or TCL_ROOT to help CMake find your Tcl installation.")
endif()

add_library(tclsoarlib MODULE
  src/main.cpp
  src/TclSoarLib.cpp
)

target_include_directories(tclsoarlib PRIVATE
  ${TCL_INCLUDE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Core/ElementXML/src
  ${CMAKE_CURRENT_SOURCE_DIR}/../Core/shared
  ${CMAKE_BINARY_DIR}/Core/shared
)

target_link_libraries(tclsoarlib PRIVATE soar_lib ${TCL_LIBRARY})

# On Windows, _USRDLL makes Export.h resolve EXPORT to __declspec(dllexport)
if(WIN32)
  target_compile_definitions(tclsoarlib PRIVATE _USRDLL)
  set_target_properties(tclsoarlib PROPERTIES PREFIX "")
  target_link_libraries(tclsoarlib PRIVATE advapi32)
endif()

if(APPLE)
  set_target_properties(tclsoarlib PROPERTIES
    SUFFIX ".dylib"
    INSTALL_RPATH "@loader_path"
  )
elseif(UNIX)
  set_target_properties(tclsoarlib PROPERTIES
    SUFFIX ".so"
    INSTALL_RPATH "$ORIGIN"
  )
endif()

# Suppress warnings in Tcl-generated code
target_compile_options(tclsoarlib PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>
  $<$<CXX_COMPILER_ID:MSVC>:/w>
)

# Install the module: on Windows, tclsoarlib.dll goes in bin/ alongside Soar.dll;
# on Unix, libtclsoarlib.{so,dylib} goes in lib/.
# tclsoarlib is a MODULE, so CMake uses LIBRARY DESTINATION on all platforms.
if(WIN32)
  install(TARGETS tclsoarlib LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  install(TARGETS tclsoarlib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install Tcl support scripts
install(DIRECTORY tcl/
  DESTINATION tcl
  FILES_MATCHING
    PATTERN "*.tcl"
    PATTERN "*.soar"
    PATTERN "tclIndex"
)
