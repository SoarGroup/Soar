# Java Debugger and supporting JARs
# Requires: SWIG_JAVA (for sml.jar), Java Development Kit
cmake_minimum_required(VERSION 3.23)

find_package(Java REQUIRED COMPONENTS Development Runtime)

set(JAVA_OUTPUT_DIR "${CMAKE_BINARY_DIR}/java")
set(SWIG_JAVA_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Core/ClientSMLSWIG/java")
set(JAVA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Path separator is different on Windows vs Unix
if(WIN32)
  set(CPSEP ";")
else()
  set(CPSEP ":")
endif()

# --- Determine platform-specific SWT JAR ---
if(WIN32)
  set(SWT_PLATFORM_DIR "win64")
elseif(APPLE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    set(SWT_PLATFORM_DIR "osx64/arm")
  else()
    set(SWT_PLATFORM_DIR "osx64/i386")
  endif()
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    set(SWT_PLATFORM_DIR "gtk64/arm")
  else()
    set(SWT_PLATFORM_DIR "gtk64/i386")
  endif()
endif()

set(SWT_JAR "${JAVA_SRC_DIR}/Debugger/lib/swt/${SWT_PLATFORM_DIR}/swt.jar")
message(STATUS "Java Debugger: Using SWT from ${SWT_PLATFORM_DIR}")

# --- External library JARs ---
set(DEBUGGER_LIB_DIR "${JAVA_SRC_DIR}/Debugger/lib")
set(SMLJAVA_LIB_DIR "${JAVA_SRC_DIR}/SMLJava/lib")

set(DEBUGGER_EXTERNAL_JARS
  "${DEBUGGER_LIB_DIR}/jcommon-1.0.10.jar"
  "${DEBUGGER_LIB_DIR}/jfreechart-1.0.6.jar"
  "${DEBUGGER_LIB_DIR}/jfreechart-1.0.6-swt.jar"
  "${DEBUGGER_LIB_DIR}/swtgraphics2d.jar"
  "${DEBUGGER_LIB_DIR}/commons-cli-1.9.0.jar"
  "${SWT_JAR}"
)

set(SMLJAVA_EXTERNAL_JARS
  "${SMLJAVA_LIB_DIR}/commons-logging-1.1.1.jar"
  "${SMLJAVA_LIB_DIR}/log4j-1.2.15.jar"
  "${SMLJAVA_LIB_DIR}/stopwatch-0.4-with-deps.jar"
)

# ============================================================
# JAR 1: soar-debugger-api.jar  (add_custom_target to avoid
#         Ninja file-level deps on sml.jar which is POST_BUILD)
# ============================================================
set(DEBUGGER_API_JAR "${JAVA_OUTPUT_DIR}/soar-debugger-api.jar")
set(DEBUGGER_API_CLASSES "${CMAKE_CURRENT_BINARY_DIR}/debugger-api-classes")

add_custom_target(build_debugger_api
  COMMAND ${CMAKE_COMMAND} -E make_directory "${DEBUGGER_API_CLASSES}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${JAVA_OUTPUT_DIR}"
  COMMAND ${Java_JAVAC_EXECUTABLE}
    --release 11
    -d "${DEBUGGER_API_CLASSES}"
    -encoding UTF-8
    -cp "${SWIG_JAVA_OUTPUT_DIR}/sml.jar"
    "${JAVA_SRC_DIR}/DebuggerAPI/src/edu/umich/soar/debugger/jmx/SoarCommandLineMXBean.java"
    "${JAVA_SRC_DIR}/DebuggerAPI/src/edu/umich/soar/debugger/jmx/SoarCommandLineClient.java"
  COMMAND ${Java_JAR_EXECUTABLE} cfm "${DEBUGGER_API_JAR}"
    "${JAVA_SRC_DIR}/DebuggerAPI/MANIFEST.MF"
    -C "${DEBUGGER_API_CLASSES}" .
  COMMENT "Building soar-debugger-api.jar"
  VERBATIM
)

# ============================================================
# JAR 2: soar-smljava.jar
# ============================================================
set(SMLJAVA_JAR "${JAVA_OUTPUT_DIR}/soar-smljava.jar")
set(SMLJAVA_CLASSES "${CMAKE_CURRENT_BINARY_DIR}/smljava-classes")

# Build classpath for SMLJava
list(JOIN SMLJAVA_EXTERNAL_JARS "${CPSEP}" SMLJAVA_EXT_CP)
set(SMLJAVA_CP "${SWIG_JAVA_OUTPUT_DIR}/sml.jar${CPSEP}${SMLJAVA_EXT_CP}")

file(GLOB_RECURSE SMLJAVA_SOURCES "${JAVA_SRC_DIR}/SMLJava/src/main/java/*.java")

add_custom_target(build_smljava
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SMLJAVA_CLASSES}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${JAVA_OUTPUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E env "JAVA_TOOL_OPTIONS=-Dfile.encoding=UTF8"
    ${Java_JAVAC_EXECUTABLE}
    --release 11
    -d "${SMLJAVA_CLASSES}"
    -encoding UTF-8
    -cp "${SMLJAVA_CP}"
    ${SMLJAVA_SOURCES}
  COMMAND ${Java_JAR_EXECUTABLE} cfm "${SMLJAVA_JAR}"
    "${JAVA_SRC_DIR}/SMLJava/MANIFEST.MF"
    -C "${SMLJAVA_CLASSES}" .
  COMMAND ${Java_JAR_EXECUTABLE} uf "${SMLJAVA_JAR}"
    -C "${JAVA_SRC_DIR}/SMLJava/resources" .
  COMMENT "Building soar-smljava.jar"
  VERBATIM
)

# ============================================================
# JAR 3: soar-debugger.jar
# ============================================================
set(DEBUGGER_JAR "${JAVA_OUTPUT_DIR}/soar-debugger.jar")
set(DEBUGGER_CLASSES "${CMAKE_CURRENT_BINARY_DIR}/debugger-classes")

# Build classpath for Debugger
list(JOIN DEBUGGER_EXTERNAL_JARS "${CPSEP}" DEBUGGER_EXT_CP)
set(DEBUGGER_CP "${SWIG_JAVA_OUTPUT_DIR}/sml.jar${CPSEP}${DEBUGGER_API_JAR}${CPSEP}${SMLJAVA_JAR}${CPSEP}${DEBUGGER_EXT_CP}")

file(GLOB_RECURSE DEBUGGER_SOURCES "${JAVA_SRC_DIR}/Debugger/src/*.java")

add_custom_target(build_debugger
  COMMAND ${CMAKE_COMMAND} -E make_directory "${DEBUGGER_CLASSES}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${JAVA_OUTPUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E env "JAVA_TOOL_OPTIONS=-Dfile.encoding=UTF8"
    ${Java_JAVAC_EXECUTABLE}
    --release 11
    -d "${DEBUGGER_CLASSES}"
    -encoding UTF-8
    -cp "${DEBUGGER_CP}"
    ${DEBUGGER_SOURCES}
  COMMAND ${Java_JAR_EXECUTABLE} cfm "${DEBUGGER_JAR}"
    "${JAVA_SRC_DIR}/Debugger/manifest1"
    -C "${DEBUGGER_CLASSES}" .
  COMMAND ${Java_JAR_EXECUTABLE} uf "${DEBUGGER_JAR}"
    -C "${JAVA_SRC_DIR}/Debugger/resources" .
  COMMENT "Building soar-debugger.jar"
  VERBATIM
)

# ============================================================
# JAR 4: SoarJavaDebugger.jar (launcher)
# ============================================================
set(LAUNCHER_JAR "${CMAKE_BINARY_DIR}/SoarJavaDebugger.jar")

add_custom_target(build_launcher
  COMMAND ${Java_JAR_EXECUTABLE} cfm "${LAUNCHER_JAR}"
    "${JAVA_SRC_DIR}/Debugger/manifest2"
  COMMENT "Building SoarJavaDebugger.jar (launcher)"
  VERBATIM
)

# ============================================================
# Dependency chain (target-level only — no file deps)
# sml.jar -> debugger_api, smljava -> debugger -> launcher
# ============================================================
if(TARGET soar_sml_java)
  add_dependencies(build_debugger_api soar_sml_java)
  add_dependencies(build_smljava soar_sml_java)
endif()
add_dependencies(build_debugger build_debugger_api build_smljava)
add_dependencies(build_launcher build_debugger)

# ============================================================
# Top-level target
# ============================================================
add_custom_target(java_debugger ALL)
add_dependencies(java_debugger build_launcher)

# ============================================================
# Copy external libraries to output
# ============================================================
foreach(jar ${DEBUGGER_EXTERNAL_JARS})
  get_filename_component(jar_name "${jar}" NAME)
  add_custom_command(TARGET java_debugger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${jar}" "${JAVA_OUTPUT_DIR}/${jar_name}"
    COMMENT "Copying ${jar_name} to output"
  )
endforeach()

# ============================================================
# Install
# ============================================================
install(FILES
  ${DEBUGGER_API_JAR}
  ${SMLJAVA_JAR}
  ${DEBUGGER_JAR}
  DESTINATION java
)

install(FILES ${LAUNCHER_JAR}
  DESTINATION .
)

# Install external dependency JARs
foreach(jar ${DEBUGGER_EXTERNAL_JARS})
  install(FILES "${jar}" DESTINATION java)
endforeach()

foreach(jar ${SMLJAVA_EXTERNAL_JARS})
  install(FILES "${jar}" DESTINATION java)
endforeach()
