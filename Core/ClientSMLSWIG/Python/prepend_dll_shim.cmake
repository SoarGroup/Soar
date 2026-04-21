# Prepend a Windows DLL-search-path shim to the SWIG-generated
# Python_sml_ClientInterface.py, so importing it on Python 3.8+ can locate
# Soar.dll (and friends) from the .pyd's own directory.
#
# Idempotent: only prepends if not already present.

if(NOT DEFINED PY_WRAPPER)
  message(FATAL_ERROR "PY_WRAPPER not set")
endif()

if(NOT EXISTS "${PY_WRAPPER}")
  message(WARNING "prepend_dll_shim: ${PY_WRAPPER} does not exist; skipping")
  return()
endif()

set(MARKER "# __soar_dll_search_shim__")

file(READ "${PY_WRAPPER}" _CONTENTS)

string(FIND "${_CONTENTS}" "${MARKER}" _idx)
if(NOT _idx EQUAL -1)
  # Already patched.
  return()
endif()

set(SHIM "${MARKER}
import os as _os
import sys as _sys
if _sys.platform == \"win32\" and hasattr(_os, \"add_dll_directory\"):
    try:
        _os.add_dll_directory(_os.path.dirname(_os.path.abspath(__file__)))
    except (OSError, FileNotFoundError):
        pass
del _os, _sys
")

file(WRITE "${PY_WRAPPER}" "${SHIM}${_CONTENTS}")
message(STATUS "Patched ${PY_WRAPPER} with DLL-search shim")
