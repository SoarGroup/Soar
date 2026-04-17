# SWIG Language Bindings for Soar SML
cmake_minimum_required(VERSION 3.23)

# Options for different language bindings
option(SWIG "Build all SWIG bindings" OFF)
option(SWIG_PYTHON "Build Python SWIG bindings" OFF)
option(SWIG_JAVA "Build Java SWIG bindings" OFF)
option(SWIG_JAVASCRIPT "Build JavaScript SWIG bindings" OFF)
option(SWIG_TCL "Build Tcl SWIG bindings" OFF)
option(SWIG_CSHARP "Build C# SWIG bindings" OFF)

if(SWIG)
    set(SWIG_PYTHON ON CACHE BOOL "Build Python SWIG bindings" FORCE)
    set(SWIG_JAVA ON CACHE BOOL "Build Java SWIG bindings" FORCE)
    set(SWIG_JAVASCRIPT ON CACHE BOOL "Build JavaScript SWIG bindings" FORCE)
    set(SWIG_TCL ON CACHE BOOL "Build Tcl SWIG bindings" FORCE)
    set(SWIG_CSHARP ON CACHE BOOL "Build C# SWIG bindings" FORCE)
endif()

# Find SWIG (required for all bindings)
if(SWIG OR SWIG_PYTHON OR SWIG_JAVA OR SWIG_JAVASCRIPT OR SWIG_TCL OR SWIG_CSHARP)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})

    # Common SWIG flags
    set(CMAKE_SWIG_FLAGS -c++)

    if(NOT SVS)
        list(APPEND CMAKE_SWIG_FLAGS -DNO_SVS)
    endif()

    # Common include directories for SWIG
    set(SWIG_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../ClientSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../ConnectionSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../ElementXML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../KernelSML/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../SoarKernel/src/shared
    )
endif()

# JavaScript bindings (via SWIG JavaScript + Node backend)
if(SWIG_JAVASCRIPT)
    message(STATUS "Soar SWIG: Building JavaScript bindings")

    find_program(NODE_EXECUTABLE node)
    if(NOT NODE_EXECUTABLE)
        message(FATAL_ERROR "SWIG_JAVASCRIPT is ON, but Node.js executable was not found. Install Node.js or disable SWIG_JAVASCRIPT.")
    endif()

    # Node.js core headers (for node_api.h)
    execute_process(
        COMMAND ${NODE_EXECUTABLE} -p "require('path').join(require('path').dirname(process.execPath), '..', 'include', 'node')"
        OUTPUT_VARIABLE NODE_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT EXISTS "${NODE_INCLUDE_DIR}/node_api.h")
        find_path(NODE_INCLUDE_DIR node_api.h
            PATHS
                /opt/homebrew/include/node
                /usr/local/include/node
                /usr/include/node
            NO_DEFAULT_PATH
        )
    endif()
    if(NOT NODE_INCLUDE_DIR)
        message(FATAL_ERROR "SWIG_JAVASCRIPT is ON, but node_api.h was not found. Install Node.js development headers.")
    endif()

    # node-addon-api headers (for napi.h used by SWIG -napi backend)
    execute_process(
        COMMAND ${NODE_EXECUTABLE} -p "try { require('node-addon-api').include } catch(e) { '' }"
        OUTPUT_VARIABLE NODE_ADDON_API_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "\"" "" NODE_ADDON_API_INCLUDE_DIR "${NODE_ADDON_API_INCLUDE_DIR}")

    if(NOT NODE_ADDON_API_INCLUDE_DIR)
        unset(NODE_ADDON_API_INCLUDE_DIR)
        unset(NODE_ADDON_API_INCLUDE_DIR CACHE)

        set(NODE_ADDON_API_HINTS
            /opt/homebrew/lib/node_modules/node-addon-api
            /usr/local/lib/node_modules/node-addon-api
            /usr/lib/node_modules/node-addon-api
            /opt/homebrew/lib/node_modules/@vscode/vsce/node_modules/node-addon-api
        )

        execute_process(
            COMMAND npm root -g
            OUTPUT_VARIABLE NODE_GLOBAL_ROOT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(NODE_GLOBAL_ROOT)
            list(APPEND NODE_ADDON_API_HINTS
                "${NODE_GLOBAL_ROOT}/node-addon-api"
                "${NODE_GLOBAL_ROOT}/@vscode/vsce/node_modules/node-addon-api"
            )
            find_path(NODE_ADDON_API_INCLUDE_DIR napi.h
                PATHS ${NODE_ADDON_API_HINTS}
                NO_DEFAULT_PATH
            )
        else()
            find_path(NODE_ADDON_API_INCLUDE_DIR napi.h
                PATHS ${NODE_ADDON_API_HINTS}
                NO_DEFAULT_PATH
            )
        endif()
    endif()

    if(NOT NODE_ADDON_API_INCLUDE_DIR)
        message(FATAL_ERROR "SWIG_JAVASCRIPT is ON, but node-addon-api headers (napi.h) were not found. Install with 'npm install -g node-addon-api' or disable SWIG_JAVASCRIPT.")
    endif()

    set(JAVASCRIPT_SWIG_FLAGS ${CMAKE_SWIG_FLAGS})
    list(APPEND JAVASCRIPT_SWIG_FLAGS -javascript -napi)

    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND JAVASCRIPT_SWIG_FLAGS -I${dir})
    endforeach()

    list(APPEND JAVASCRIPT_SWIG_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/JS)
    list(APPEND JAVASCRIPT_SWIG_FLAGS -I${NODE_INCLUDE_DIR})
    list(APPEND JAVASCRIPT_SWIG_FLAGS -I${NODE_ADDON_API_INCLUDE_DIR})

    set(JAVASCRIPT_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/javascript")
    set(JAVASCRIPT_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/JS/JavaScript_sml_ClientInterface.i")
    set(JAVASCRIPT_COMMON_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sml_ClientInterface.i")
    set(JAVASCRIPT_WRAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/JavaScript_sml_ClientInterfaceJAVASCRIPT_wrap.cxx")

    add_custom_command(
        OUTPUT ${JAVASCRIPT_WRAP_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${JAVASCRIPT_OUTPUT_DIR}"
        COMMAND ${SWIG_EXECUTABLE} ${JAVASCRIPT_SWIG_FLAGS}
                -outdir "${JAVASCRIPT_OUTPUT_DIR}"
                -o "${JAVASCRIPT_WRAP_FILE}"
                "${JAVASCRIPT_INTERFACE_FILE}"
        DEPENDS ${JAVASCRIPT_INTERFACE_FILE} ${JAVASCRIPT_COMMON_INTERFACE_FILE}
        COMMENT "Generating JavaScript SWIG wrapper sources"
        VERBATIM
    )

    add_custom_target(soar_sml_javascript ALL
        DEPENDS ${JAVASCRIPT_WRAP_FILE}
    )

    add_library(soar_sml_javascript_node MODULE ${JAVASCRIPT_WRAP_FILE})
    add_dependencies(soar_sml_javascript_node soar_sml_javascript)

    target_link_libraries(soar_sml_javascript_node PRIVATE soar_lib)
    target_include_directories(soar_sml_javascript_node PRIVATE
        ${NODE_INCLUDE_DIR}
        ${NODE_ADDON_API_INCLUDE_DIR}
        ${SWIG_INCLUDE_DIRS}
    )
    target_compile_features(soar_sml_javascript_node PRIVATE cxx_std_20)

    set_target_properties(soar_sml_javascript_node PROPERTIES
        OUTPUT_NAME "JavaScript_sml_ClientInterface"
        PREFIX ""
        SUFFIX ".node"
        LIBRARY_OUTPUT_DIRECTORY "${JAVASCRIPT_OUTPUT_DIR}"
    )

    if(APPLE)
        target_link_options(soar_sml_javascript_node PRIVATE -undefined dynamic_lookup)
    endif()

    install(FILES
        ${JAVASCRIPT_WRAP_FILE}
        DESTINATION javascript
        OPTIONAL
    )

    install(TARGETS soar_sml_javascript_node
        LIBRARY DESTINATION javascript
        RUNTIME DESTINATION javascript
    )

    install(FILES
        JS/JavaScript_sml_ClientInterface.js
        DESTINATION javascript
    )
endif()

# Python bindings
if(SWIG_PYTHON)
    message(STATUS "Soar SWIG: Building Python bindings")

    # Find Python
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

    # Configure SWIG for Python
    set_property(SOURCE Python/Python_sml_ClientInterface.i PROPERTY CPLUSPLUS ON)

    # Set Python-specific SWIG flags (language flag added automatically by swig_add_library)
    set(PYTHON_SWIG_FLAGS ${CMAKE_SWIG_FLAGS})

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND PYTHON_SWIG_FLAGS -I${dir})
    endforeach()

    # Create the SWIG module
    swig_add_library(soar_sml_python
        TYPE MODULE
        LANGUAGE python
        SOURCES Python/Python_sml_ClientInterface.i
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/python"
        OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Set Python-specific SWIG flags for this target
    set_property(TARGET soar_sml_python PROPERTY SWIG_COMPILE_OPTIONS ${PYTHON_SWIG_FLAGS})

    # Set properties for the SWIG module
    set_target_properties(soar_sml_python PROPERTIES
        OUTPUT_NAME "Python_sml_ClientInterface"
    )

    # Link against soar library and Python
    target_link_libraries(soar_sml_python soar_lib Python3::Python)

    # Include directories
    target_include_directories(soar_sml_python PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${SWIG_INCLUDE_DIRS}
    )

    # Platform-specific settings
    if(UNIX AND NOT APPLE)
        target_compile_options(soar_sml_python PRIVATE -Wno-unused -fno-strict-aliasing)
    endif()

    if(APPLE)
        set_target_properties(soar_sml_python PROPERTIES
            SUFFIX ".so"
        )
    endif()

    # Install Python files
    install(TARGETS soar_sml_python
        LIBRARY DESTINATION python
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/python/Python_sml_ClientInterface.py"
        DESTINATION python
    )

    install(FILES "Python/TestPythonSML.py"
        DESTINATION python
    )
endif()

# Java bindings
if(SWIG_JAVA)
    message(STATUS "Soar SWIG: Building Java bindings")

    # Find Java and JNI
    find_package(Java REQUIRED COMPONENTS Development Runtime)
    find_package(JNI REQUIRED)

    # Configure SWIG for Java
    set_property(SOURCE Java/Java_sml_ClientInterface.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE Java/Java_sml_ClientInterface.i PROPERTY SWIG_MODULE_NAME sml)

    # Set Java-specific SWIG flags (language flag added automatically by swig_add_library)
    set(JAVA_SWIG_FLAGS ${CMAKE_SWIG_FLAGS})

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND JAVA_SWIG_FLAGS -I${dir})
    endforeach()

    # Add Java directory to SWIG flags
    list(APPEND JAVA_SWIG_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/Java)
    list(APPEND JAVA_SWIG_FLAGS -package sml)

    # Copy JavaCallbackByHand.h to the build directory
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Java/JavaCallbackByHand.h"
        "${CMAKE_CURRENT_BINARY_DIR}/java/JavaCallbackByHand.h"
        COPYONLY
    )

    # Create the SWIG module
    swig_add_library(soar_sml_java
        TYPE SHARED
        LANGUAGE java
        SOURCES Java/Java_sml_ClientInterface.i
        OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/java"
        OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Set Java-specific SWIG flags for this target
    set_property(TARGET soar_sml_java PROPERTY SWIG_COMPILE_OPTIONS ${JAVA_SWIG_FLAGS})

    # Set properties for the SWIG module
    set_target_properties(soar_sml_java PROPERTIES
        OUTPUT_NAME "Java_sml_ClientInterface"
    )

    # On macOS, use .jnilib extension for JNI libraries
    if(APPLE)
        set_target_properties(soar_sml_java PROPERTIES
            SUFFIX ".jnilib"
        )
    endif()

    # Link against soar library and JNI
    target_link_libraries(soar_sml_java soar_lib ${JNI_LIBRARIES})

    # Include directories
    target_include_directories(soar_sml_java PRIVATE
        ${JNI_INCLUDE_DIRS}
        ${SWIG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Java
        ${CMAKE_CURRENT_BINARY_DIR}/java
    )

    # Platform-specific settings
    if(UNIX AND NOT APPLE)
        target_compile_options(soar_sml_java PRIVATE -Wno-unused -fno-strict-aliasing)
    endif()

    # Compile Java files and create JAR (cross-platform)
    add_custom_command(
        TARGET soar_sml_java POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/java/classes"
        COMMAND ${Java_JAVAC_EXECUTABLE} -d "${CMAKE_CURRENT_BINARY_DIR}/java/classes"
                "${CMAKE_CURRENT_BINARY_DIR}/java/*.java"
        COMMAND ${Java_JAR_EXECUTABLE} cf "${CMAKE_CURRENT_BINARY_DIR}/java/sml.jar"
                -C "${CMAKE_CURRENT_BINARY_DIR}/java/classes" .
        COMMENT "Compiling Java files and creating sml.jar"
        VERBATIM
    )

    # Install Java files
    install(TARGETS soar_sml_java
        LIBRARY DESTINATION java
        RUNTIME DESTINATION java
        ARCHIVE DESTINATION java
    )

    # Install JAR file
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/java/sml.jar"
        DESTINATION java
    )
endif()

# Tcl bindings
if(SWIG_TCL)
    message(STATUS "Soar SWIG: Building Tcl bindings")

    # Find only Tcl (not Tk) — FindTCL's REQUIRED checks for both TCL+TK
    find_package(TCL)
    if(NOT TCL_FOUND)
        message(FATAL_ERROR "Could not find Tcl development files (TCL_LIBRARY and TCL_INCLUDE_PATH). "
            "Set CMAKE_PREFIX_PATH or TCL_ROOT to help CMake find your Tcl installation.")
    endif()

    # Set Tcl-specific SWIG flags
    set(TCL_SWIG_FLAGS ${CMAKE_SWIG_FLAGS})

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND TCL_SWIG_FLAGS -I${dir})
    endforeach()

    list(APPEND TCL_SWIG_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/Tcl)

    set(TCL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/tcl")
    set(TCL_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Tcl/Tcl_sml_ClientInterface.i")
    set(TCL_COMMON_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sml_ClientInterface.i")
    set(TCL_WRAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/Tcl_sml_ClientInterface_wrap.cpp")

    add_custom_command(
        OUTPUT ${TCL_WRAP_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${TCL_OUTPUT_DIR}"
        COMMAND ${SWIG_EXECUTABLE} ${TCL_SWIG_FLAGS}
                -tcl
                -pkgversion ${PROJECT_VERSION}
                -o "${TCL_WRAP_FILE}"
                "${TCL_INTERFACE_FILE}"
        DEPENDS ${TCL_INTERFACE_FILE} ${TCL_COMMON_INTERFACE_FILE}
        COMMENT "Generating Tcl SWIG wrapper sources"
        VERBATIM
    )

    add_library(soar_sml_tcl MODULE ${TCL_WRAP_FILE})

    target_link_libraries(soar_sml_tcl PRIVATE soar_lib ${TCL_LIBRARY})
    target_include_directories(soar_sml_tcl PRIVATE
        ${TCL_INCLUDE_PATH}
        ${SWIG_INCLUDE_DIRS}
    )

    set_target_properties(soar_sml_tcl PROPERTIES
        OUTPUT_NAME "Tcl_sml_ClientInterface"
    )

    # Windows: no lib prefix; Unix: keep default lib prefix
    if(WIN32)
        set_target_properties(soar_sml_tcl PROPERTIES PREFIX "")
    endif()

    if(APPLE)
        set_target_properties(soar_sml_tcl PROPERTIES SUFFIX ".dylib")
    elseif(UNIX)
        set_target_properties(soar_sml_tcl PROPERTIES SUFFIX ".so")
    endif()

    # Determine platform for pkgIndex.tcl
    if(WIN32)
        set(TCL_PLATFORM "win")
    elseif(APPLE)
        set(TCL_PLATFORM "mac")
    else()
        set(TCL_PLATFORM "linux")
    endif()

    # Copy platform-specific pkgIndex.tcl
    add_custom_command(TARGET soar_sml_tcl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/Tcl/pkgIndex_${TCL_PLATFORM}.tcl"
            "${TCL_OUTPUT_DIR}/pkgIndex.tcl"
        COMMENT "Copying pkgIndex_${TCL_PLATFORM}.tcl"
    )

    # Copy the shared library to the tcl output directory
    add_custom_command(TARGET soar_sml_tcl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:soar_sml_tcl>"
            "${TCL_OUTPUT_DIR}/"
        COMMENT "Copying Tcl SWIG module to output directory"
    )

    install(TARGETS soar_sml_tcl
        LIBRARY DESTINATION tcl
        RUNTIME DESTINATION tcl
    )

    install(FILES "${TCL_OUTPUT_DIR}/pkgIndex.tcl"
        DESTINATION tcl
    )

    install(FILES "Tcl/TestTclSML.tcl"
        DESTINATION tcl
    )
endif()

# C# bindings
if(SWIG_CSHARP)
    message(STATUS "Soar SWIG: Building C# bindings")

    find_program(DOTNET_EXECUTABLE dotnet)
    if(NOT DOTNET_EXECUTABLE)
        message(WARNING "SWIG_CSHARP is ON, but dotnet executable was not found. Native wrapper will be built but managed assembly requires dotnet.")
    endif()

    # Set C#-specific SWIG flags
    set(CSHARP_SWIG_FLAGS ${CMAKE_SWIG_FLAGS})

    # Add include directories to SWIG flags
    foreach(dir ${SWIG_INCLUDE_DIRS})
        list(APPEND CSHARP_SWIG_FLAGS -I${dir})
    endforeach()

    list(APPEND CSHARP_SWIG_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/CSharp)

    set(CSHARP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/csharp/src")
    set(CSHARP_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CSharp/CSharp_sml_ClientInterface.i")
    set(CSHARP_COMMON_INTERFACE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sml_ClientInterface.i")
    set(CSHARP_WRAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/CSharp_sml_ClientInterface_wrap.cpp")

    add_custom_command(
        OUTPUT ${CSHARP_WRAP_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CSHARP_OUTPUT_DIR}"
        COMMAND ${SWIG_EXECUTABLE} ${CSHARP_SWIG_FLAGS}
                -csharp
                -namespace sml
                -dllimport CSharp_sml_ClientInterface
                -outdir "${CSHARP_OUTPUT_DIR}"
                -o "${CSHARP_WRAP_FILE}"
                "${CSHARP_INTERFACE_FILE}"
        DEPENDS ${CSHARP_INTERFACE_FILE} ${CSHARP_COMMON_INTERFACE_FILE}
        COMMENT "Generating C# SWIG wrapper sources"
        VERBATIM
    )

    # Build the native wrapper library
    add_library(soar_sml_csharp MODULE ${CSHARP_WRAP_FILE})

    target_link_libraries(soar_sml_csharp PRIVATE soar_lib)
    target_include_directories(soar_sml_csharp PRIVATE
        ${SWIG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/CSharp
    )

    set_target_properties(soar_sml_csharp PROPERTIES
        OUTPUT_NAME "CSharp_sml_ClientInterface"
    )

    # Windows: no lib prefix; Unix: keep default lib prefix
    if(WIN32)
        set_target_properties(soar_sml_csharp PROPERTIES PREFIX "")
    endif()

    # Copy native library to csharp output
    set(CSHARP_LIB_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/csharp")
    add_custom_command(TARGET soar_sml_csharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:soar_sml_csharp>"
            "${CSHARP_LIB_OUTPUT_DIR}/"
        COMMENT "Copying C# native wrapper to output directory"
    )

    # Build the managed assembly if dotnet is available
    if(DOTNET_EXECUTABLE)
        set(CSHARP_CSPROJ "${CSHARP_LIB_OUTPUT_DIR}/sml_csharp.csproj")
        file(WRITE "${CSHARP_CSPROJ}"
"<Project Sdk=\"Microsoft.NET.Sdk\">
  <PropertyGroup>
    <TargetFramework>net6.0</TargetFramework>
    <OutputType>Library</OutputType>
    <AssemblyName>sml_csharp</AssemblyName>
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
  </PropertyGroup>
  <ItemGroup>
    <Compile Include=\"src/**/*.cs\" />
  </ItemGroup>
</Project>
")

        add_custom_command(TARGET soar_sml_csharp POST_BUILD
            COMMAND ${DOTNET_EXECUTABLE} build "${CSHARP_CSPROJ}"
                    -o "${CSHARP_LIB_OUTPUT_DIR}" -c Release --nologo -v quiet
            COMMENT "Building C# managed assembly (sml_csharp.dll)"
            VERBATIM
        )
    endif()

    install(TARGETS soar_sml_csharp
        LIBRARY DESTINATION csharp
        RUNTIME DESTINATION csharp
    )

    if(DOTNET_EXECUTABLE)
        install(FILES "${CSHARP_LIB_OUTPUT_DIR}/sml_csharp.dll"
            DESTINATION csharp
            OPTIONAL
        )
    endif()
endif()
