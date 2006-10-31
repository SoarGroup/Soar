# tkConfig.sh --
# 
# This shell script (for sh) is generated automatically by Tk's
# configure script.  It will create shell variables for most of
# the configuration options discovered by the configure script.
# This script is intended to be included by the configure scripts
# for Tk extensions so that they don't have to figure this all
# out for themselves.  This file does not duplicate information
# already provided by tclConfig.sh, so you may need to use that
# file in addition to this one.
#
# The information in this file is specific to a single platform.
#
# RCS: @(#) $Id: tkConfig.sh.in,v 1.2 2001/10/15 21:19:16 hobbs Exp $

# Tk's version number.
TK_VERSION='8.4'
TK_MAJOR_VERSION='8'
TK_MINOR_VERSION='4'
TK_PATCH_LEVEL='.11'

# -D flags for use with the C compiler.
TK_DEFS=' -DTCL_THREADS=1 -DUSE_THREAD_ALLOC=1 '

# Flag, 1: we built a shared lib, 0 we didn't
TK_SHARED_BUILD=1

# This indicates if Tk was build with debugging symbols
TK_DBGX=

# The name of the Tk library (may be either a .a file or a shared library):
TK_LIB_FILE='tk84.lib'

# Additional libraries to use when linking Tk.
TK_LIBS='user32.lib advapi32.lib gdi32.lib comdlg32.lib imm32.lib comctl32.lib shell32.lib'

# Top-level directory in which Tcl's platform-independent files are
# installed.
TK_PREFIX='C:/Tcl'

# Top-level directory in which Tcl's platform-specific files (e.g.
# executables) are installed.
TK_EXEC_PREFIX='C:/Tcl'

# -l flag to pass to the linker to pick up the Tcl library
TK_LIB_FLAG='-ltk84'

# String to pass to linker to pick up the Tk library from its
# build directory.
TK_BUILD_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tk/win/win32-ix86 -ltk84'

# String to pass to linker to pick up the Tk library from its
# installed directory.
TK_LIB_SPEC='-LC:/Tcl/lib -ltk84'

# Location of the top-level source directory from which Tk was built.
# This is the directory that contains a README file as well as
# subdirectories such as generic, unix, etc.  If Tk was compiled in a
# different place than the directory containing the source files, this
# points to the location of the sources, not the location where Tk was
# compiled.
TK_SRC_DIR='/home/Administrator/dbn/lba/night/builds/win32-ix86/tk'

# Needed if you want to make a 'fat' shared library library
# containing tk objects or link a different wish.
TK_CC_SEARCH_FLAGS=''
TK_LD_SEARCH_FLAGS=''

# The name of the Tk stub library (.a):
TK_STUB_LIB_FILE='tkstub84.lib'

# -l flag to pass to the linker to pick up the Tk stub library
TK_STUB_LIB_FLAG='-ltkstub84'

# String to pass to linker to pick up the Tk stub library from its
# build directory.
TK_BUILD_STUB_LIB_SPEC='-L/home/Administrator/dbn/lba/night/builds/win32-ix86/tk/win/win32-ix86 -ltkstub84'

# String to pass to linker to pick up the Tk stub library from its
# installed directory.
TK_STUB_LIB_SPEC='-LC:/Tcl/lib -ltkstub84'

# Path to the Tk stub library in the build directory.
TK_BUILD_STUB_LIB_PATH='/home/Administrator/dbn/lba/night/builds/win32-ix86/tk/win/win32-ix86/tkstub84.lib'

# Path to the Tk stub library in the install directory.
TK_STUB_LIB_PATH='C:/Tcl/lib/tkstub84.lib'
