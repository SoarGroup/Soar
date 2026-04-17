# Assemble a Soar multiplatform release zip from per-platform cmake install
# trees and cloned external repos.
#
# Usage:
#   cmake -P scripts/assemble-release.cmake -- <version> <platforms-dir> [repos-dir]
#
# <platforms-dir> must contain subdirectories named:
#   soar-win_x86-64  soar-linux_x86-64  soar-mac_x86-64  soar-mac_ARM64
# Each is a cmake --install prefix tree (bin/ lib/ include/ java/ tcl/ ...).
#
# [repos-dir] (optional) contains clones of SoarGroup repos:
#   Agents  Release-Support  VisualSoar  Domains-Eaters-TankSoar

cmake_minimum_required(VERSION 3.20)

# ── Parse arguments ──────────────────────────────────────────────────────────
# cmake -P strips everything before "--", so CMAKE_ARGV{N} has:
#   0=cmake, 1=-P, 2=<script>, 3=--, 4=<version>, 5=<platforms-dir>, ...

# Find the "--" separator and grab the args after it
set(_found_sep FALSE)
set(_user_args)
math(EXPR _last "${CMAKE_ARGC} - 1")
foreach(_i RANGE 0 ${_last})
  if(_found_sep)
    list(APPEND _user_args "${CMAKE_ARGV${_i}}")
  elseif("${CMAKE_ARGV${_i}}" STREQUAL "--")
    set(_found_sep TRUE)
  endif()
endforeach()

list(LENGTH _user_args _nargs)
if(_nargs LESS 2)
  message(FATAL_ERROR
    "Usage: cmake -P scripts/assemble-release.cmake -- <version> <platforms-dir> [repos-dir]")
endif()

list(GET _user_args 0 VERSION)
list(GET _user_args 1 PLATFORMS_DIR)
if(_nargs GREATER_EQUAL 3)
  list(GET _user_args 2 REPOS_DIR)
else()
  set(REPOS_DIR "")
endif()

cmake_path(ABSOLUTE_PATH PLATFORMS_DIR)
if(REPOS_DIR)
  cmake_path(ABSOLUTE_PATH REPOS_DIR)
endif()

set(SUITE "SoarSuite_${VERSION}-Multiplatform")

message(STATUS "Version:       ${VERSION}")
message(STATUS "Platforms dir: ${PLATFORMS_DIR}")
message(STATUS "Repos dir:     ${REPOS_DIR}")
message(STATUS "Output:        ${SUITE}/")

# ── Helpers ──────────────────────────────────────────────────────────────────

# copy_if_exists(src dest) — copy a single file, silently skip if missing
function(copy_if_exists SRC DEST)
  if(EXISTS "${SRC}")
    message(STATUS "  ${SRC} -> ${DEST}")
    file(COPY "${SRC}" DESTINATION "${DEST}")
  endif()
endfunction()

# copy_glob(pattern dest) — copy all files matching a glob pattern
function(copy_glob PATTERN DEST)
  file(GLOB _files "${PATTERN}")
  foreach(_f IN LISTS _files)
    file(COPY "${_f}" DESTINATION "${DEST}")
  endforeach()
endfunction()

# copy_matching(dir pattern dest [EXCLUDE_PATTERNS ...])
# Copy files from dir matching pattern, optionally excluding some patterns.
function(copy_matching DIR PATTERN DEST)
  set(_excludes ${ARGN})
  file(GLOB _files "${DIR}/${PATTERN}")
  foreach(_f IN LISTS _files)
    cmake_path(GET _f FILENAME _name)
    set(_skip FALSE)
    foreach(_ex IN LISTS _excludes)
      if(_name MATCHES "${_ex}")
        set(_skip TRUE)
        break()
      endif()
    endforeach()
    if(NOT _skip)
      message(STATUS "  ${_f} -> ${DEST}")
      file(COPY "${_f}" DESTINATION "${DEST}")
    endif()
  endforeach()
endfunction()

# ── Clean output ─────────────────────────────────────────────────────────────

if(EXISTS "${SUITE}")
  file(REMOVE_RECURSE "${SUITE}")
endif()

set(PLATFORM_IDS win_x86-64 linux_x86-64 mac_x86-64 mac_ARM64)

# ── Per-platform binaries → bin/<platform>/ ──────────────────────────────────

set(_any_platform "")

foreach(PLAT IN LISTS PLATFORM_IDS)
  set(SRC "${PLATFORMS_DIR}/soar-${PLAT}")
  set(DEST "${SUITE}/bin/${PLAT}")
  file(MAKE_DIRECTORY "${DEST}")

  if(NOT IS_DIRECTORY "${SRC}")
    message(STATUS "WARNING: Platform ${PLAT} not found at ${SRC}, skipping")
    continue()
  endif()

  if(NOT _any_platform)
    set(_any_platform "${SRC}")
  endif()

  message(STATUS "=== Assembling ${PLAT} ===")

  # CLI executable + tests
  copy_if_exists("${SRC}/bin/soar"          "${DEST}")
  copy_if_exists("${SRC}/bin/soar.exe"      "${DEST}")
  copy_if_exists("${SRC}/bin/UnitTests"     "${DEST}")
  copy_if_exists("${SRC}/bin/UnitTests.exe" "${DEST}")

  # Core shared library
  # Windows: Soar.dll (bin/) + Soar.lib (lib/)
  copy_if_exists("${SRC}/bin/Soar.dll" "${DEST}")
  copy_if_exists("${SRC}/lib/Soar.lib" "${DEST}")
  # Unix: libSoar.so or libSoar.dylib (lib/)
  copy_matching("${SRC}/lib" "libSoar.*" "${DEST}" "\\.cmake$")

  # Java JNI library
  # Windows: java/Java_sml_ClientInterface.dll
  # Unix:    java/libJava_sml_ClientInterface.{so,jnilib}
  copy_matching("${SRC}/java" "*Java_sml_ClientInterface*" "${DEST}"
    "\\.java$" "\\.jar$")

  # Python SWIG binding
  copy_matching("${SRC}/python" "_Python_sml_ClientInterface*" "${DEST}")
  copy_if_exists("${SRC}/python/Python_sml_ClientInterface.py" "${DEST}")

  # Tcl SWIG binding
  copy_matching("${SRC}/tcl" "*Tcl_sml_ClientInterface*" "${DEST}")
  copy_if_exists("${SRC}/tcl/pkgIndex.tcl" "${DEST}")

  # TclSoarLib
  # Windows: tclsoarlib.dll (bin/), Unix: libtclsoarlib.{so,dylib} (lib/)
  copy_if_exists("${SRC}/bin/tclsoarlib.dll" "${DEST}")
  copy_matching("${SRC}/lib" "libtclsoarlib.*" "${DEST}")

  # C# SWIG binding
  copy_matching("${SRC}/csharp" "*CSharp_sml_ClientInterface*" "${DEST}")
  copy_if_exists("${SRC}/csharp/sml_csharp.dll" "${DEST}")

  # Platform-specific SWT jar
  copy_if_exists("${SRC}/java/swt.jar" "${DEST}")
endforeach()

if(NOT _any_platform)
  message(FATAL_ERROR "No platform builds found in ${PLATFORMS_DIR}")
endif()

# ── Cross-platform resources (from first available platform) ─────────────────

message(STATUS "=== Cross-platform resources (from ${_any_platform}) ===")

# Headers
file(MAKE_DIRECTORY "${SUITE}/include")
if(IS_DIRECTORY "${_any_platform}/include")
  file(COPY "${_any_platform}/include/" DESTINATION "${SUITE}/include")
endif()

# TclSoarLib support scripts
if(IS_DIRECTORY "${_any_platform}/tcl")
  file(MAKE_DIRECTORY "${SUITE}/bin/tcl")
  copy_glob("${_any_platform}/tcl/*.tcl"  "${SUITE}/bin/tcl")
  copy_glob("${_any_platform}/tcl/*.soar" "${SUITE}/bin/tcl")
  copy_if_exists("${_any_platform}/tcl/tclIndex" "${SUITE}/bin/tcl")
endif()

# Java JARs
file(MAKE_DIRECTORY "${SUITE}/bin/java")
set(_jars
  sml.jar soar-debugger.jar soar-debugger-api.jar soar-smljava.jar
  jcommon-1.0.10.jar jfreechart-1.0.6.jar jfreechart-1.0.6-swt.jar
  swtgraphics2d.jar commons-cli-1.9.0.jar commons-logging-1.1.1.jar
  log4j-1.2.15.jar stopwatch-0.4-with-deps.jar
)
foreach(_jar IN LISTS _jars)
  if(EXISTS "${_any_platform}/java/${_jar}")
    file(COPY "${_any_platform}/java/${_jar}" DESTINATION "${SUITE}/bin/java")
  else()
    message(STATUS "  note: ${_jar} not found (optional)")
  endif()
endforeach()

# SoarJavaDebugger launcher + settings
copy_if_exists("${_any_platform}/SoarJavaDebugger.jar" "${SUITE}/bin")
copy_if_exists("${_any_platform}/settings.soar"         "${SUITE}/bin")

# ── External repos ───────────────────────────────────────────────────────────

if(REPOS_DIR AND IS_DIRECTORY "${REPOS_DIR}")
  message(STATUS "=== External repos ===")

  # Agents
  if(IS_DIRECTORY "${REPOS_DIR}/Agents")
    file(MAKE_DIRECTORY "${SUITE}/Agents")
    file(GLOB _agent_dirs "${REPOS_DIR}/Agents/*/")
    foreach(_d IN LISTS _agent_dirs)
      cmake_path(GET _d FILENAME _name)
      if(_name STREQUAL ".git")
        continue()
      endif()
      file(COPY "${_d}" DESTINATION "${SUITE}/Agents")
    endforeach()
    copy_if_exists("${REPOS_DIR}/Release-Support/txt/Agents_readme.md"
      "${SUITE}/Agents")
    # Rename to readme.md
    if(EXISTS "${SUITE}/Agents/Agents_readme.md")
      file(RENAME "${SUITE}/Agents/Agents_readme.md" "${SUITE}/Agents/readme.md")
    endif()
  endif()

  # Eaters & TankSoar agents + configs
  if(IS_DIRECTORY "${REPOS_DIR}/Domains-Eaters-TankSoar")
    if(IS_DIRECTORY "${REPOS_DIR}/Domains-Eaters-TankSoar/agents/eaters")
      file(COPY "${REPOS_DIR}/Domains-Eaters-TankSoar/agents/eaters"
        DESTINATION "${SUITE}/Agents")
    endif()
    if(IS_DIRECTORY "${REPOS_DIR}/Domains-Eaters-TankSoar/agents/tanksoar")
      file(COPY "${REPOS_DIR}/Domains-Eaters-TankSoar/agents/tanksoar"
        DESTINATION "${SUITE}/Agents")
    endif()
    file(MAKE_DIRECTORY "${SUITE}/bin/games/maps")
    copy_glob("${REPOS_DIR}/Domains-Eaters-TankSoar/config/*.cnf"
      "${SUITE}/bin/games")
    if(IS_DIRECTORY "${REPOS_DIR}/Domains-Eaters-TankSoar/config/maps/eaters")
      file(COPY "${REPOS_DIR}/Domains-Eaters-TankSoar/config/maps/eaters"
        DESTINATION "${SUITE}/bin/games/maps")
    endif()
    if(IS_DIRECTORY "${REPOS_DIR}/Domains-Eaters-TankSoar/config/maps/tanksoar")
      file(COPY "${REPOS_DIR}/Domains-Eaters-TankSoar/config/maps/tanksoar"
        DESTINATION "${SUITE}/bin/games/maps")
    endif()
  endif()

  # VisualSoar
  file(GLOB_RECURSE _vs_jar "${REPOS_DIR}/VisualSoar/build/libs/VisualSoar*.jar")
  if(_vs_jar)
    list(GET _vs_jar 0 _vs_jar_path)
    file(COPY "${_vs_jar_path}" DESTINATION "${SUITE}/bin")
    # Rename to VisualSoar.jar (strip version suffix if present)
    cmake_path(GET _vs_jar_path FILENAME _vs_jar_name)
    if(NOT _vs_jar_name STREQUAL "VisualSoar.jar")
      file(RENAME "${SUITE}/bin/${_vs_jar_name}" "${SUITE}/bin/VisualSoar.jar")
    endif()
  endif()
  # Jackson JARs from VisualSoar
  foreach(_prefix jackson-annotations jackson-core jackson-databind)
    file(GLOB _jjars "${REPOS_DIR}/VisualSoar/lib/${_prefix}*.jar")
    foreach(_j IN LISTS _jjars)
      file(COPY "${_j}" DESTINATION "${SUITE}/bin/java")
    endforeach()
  endforeach()

  # Eaters_TankSoar.jar (pre-built)
  copy_if_exists("${REPOS_DIR}/Release-Support/SoarShuffler/jars/Eaters_TankSoar.jar"
    "${SUITE}/bin")

  # Documentation
  file(MAKE_DIRECTORY "${SUITE}/Documentation/SoarTutorial")
  copy_if_exists("${REPOS_DIR}/Release-Support/pdf/SoarManual.pdf"
    "${SUITE}/Documentation")
  file(GLOB_RECURSE _vs_manual
    "${REPOS_DIR}/VisualSoar/**/VisualSoar_UsersManual.pdf")
  if(_vs_manual)
    list(GET _vs_manual 0 _vs_manual_path)
    file(COPY "${_vs_manual_path}" DESTINATION "${SUITE}/Documentation")
  endif()
  file(GLOB _tutorials "${REPOS_DIR}/Release-Support/pdf/Soar Tutorial*")
  foreach(_t IN LISTS _tutorials)
    file(COPY "${_t}" DESTINATION "${SUITE}/Documentation/SoarTutorial")
  endforeach()

  # Launcher scripts
  foreach(_ext bat sh command)
    foreach(_script SoarCLI SoarJavaDebugger VisualSoar Eaters TankSoar setup)
      copy_if_exists(
        "${REPOS_DIR}/Release-Support/scripts/${_script}.${_ext}"
        "${SUITE}")
    endforeach()
  endforeach()

  # Root text files
  copy_if_exists("${REPOS_DIR}/Release-Support/txt/README.md"          "${SUITE}")
  copy_if_exists("${REPOS_DIR}/Release-Support/txt/Building_Soar.md"   "${SUITE}")
  copy_if_exists("${REPOS_DIR}/Release-Support/txt/license.txt"        "${SUITE}")
  copy_if_exists(
    "${REPOS_DIR}/Release-Support/txt/Release_Notes_${VERSION}.md"
    "${SUITE}")
endif()

# ── Finalize ─────────────────────────────────────────────────────────────────

# Set execute permissions on Unix (no-op on Windows)
if(NOT WIN32)
  file(GLOB_RECURSE _scripts "${SUITE}/*.sh" "${SUITE}/*.command")
  foreach(_s IN LISTS _scripts)
    file(CHMOD "${_s}" PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE)
  endforeach()
  foreach(_exe soar UnitTests)
    foreach(_plat IN LISTS PLATFORM_IDS)
      set(_path "${SUITE}/bin/${_plat}/${_exe}")
      if(EXISTS "${_path}")
        file(CHMOD "${_path}" PERMISSIONS
          OWNER_READ OWNER_WRITE OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
      endif()
    endforeach()
  endforeach()
endif()

# Create zip
file(ARCHIVE_CREATE
  OUTPUT "${SUITE}.zip"
  PATHS "${SUITE}"
  FORMAT zip
)

# Summary
file(SIZE "${SUITE}.zip" _zip_size)
math(EXPR _zip_mb "${_zip_size} / 1048576")
message(STATUS "")
message(STATUS "=== Release assembly complete ===")
message(STATUS "${SUITE}.zip  (${_zip_mb} MB)")
file(GLOB _topdirs LIST_DIRECTORIES true "${SUITE}/*/")
message(STATUS "Contents:")
foreach(_d IN LISTS _topdirs)
  if(IS_DIRECTORY "${_d}")
    cmake_path(GET _d FILENAME _name)
    file(GLOB _children LIST_DIRECTORIES true "${_d}/*")
    list(LENGTH _children _count)
    message(STATUS "  ${_name}/  (${_count} items)")
    # Show subdirs only (not individual files)
    foreach(_sd IN LISTS _children)
      if(IS_DIRECTORY "${_sd}")
        cmake_path(GET _sd FILENAME _sname)
        message(STATUS "    ${_sname}/")
      endif()
    endforeach()
  endif()
endforeach()
