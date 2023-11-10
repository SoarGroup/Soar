from dataclasses import dataclass
from pathlib import Path
import os
import subprocess
import sys

NO_GLFW_MSG = "GLFW could not be found"


@dataclass
class TclInstallInfo:
    home: Path
    lib_dir: Path
    include_dir: Path
    # the name of the library file
    dyn_lib_name: str
    # The name of the library to use in -l... directives
    include_lib_name: str
    using_framework: bool = False

    def is_valid(self):
        if not self.lib_dir.exists():
            return False, f'lib dir does not exist: "{self.lib_dir}"'
        if not (self.lib_dir / self.dyn_lib_name).exists():
            return (
                False,
                f'lib dir does not contain {self.dyn_lib_name}: "{self.lib_dir}"',
            )
        if not self.include_dir.exists():
            return False, f'include dir does not exist: "{self.include_dir}"'
        if not (self.include_dir / "tcl.h").exists():
            return False, f'include dir does not contain tcl.h: "{self.include_dir}"'
        return True, ""


def __get_dir_from_brew() -> str:
    try:
        return (
            subprocess.check_output(["brew", "--prefix", "glfw"]).decode().strip()
        )
    except subprocess.CalledProcessError:
        print(f"{env['INDENT']}glfw package not brew-installed; try `brew install glfw`")
        return None


def __prepare_for_compilation_mac(env, local_compiled_dir=None) -> TclInstallInfo:
    if not local_compiled_dir:
        local_compiled_dir = __get_dir_from_brew()
    if not local_compiled_dir:
        return False

    try:
        env.ParseConfig(f"pkg-config {local_compiled_dir}/lib/pkgconfig/glfw3.pc --libs --cflags")
    except OSError as e:
        print(
            f"{env['INDENT']}pkg-config failed. Don't forget to brew install glfw and "
            f"pkg-config! Original error: {e}"
        )
        return False

    # Tell GLFW which windowing system to use
    env.Append(CPPFLAGS='-D_GLFW_COCOA')
    # osx uses opengl as a framework instead of libraries
    for f in [ 'Cocoa', 'IOKit', 'CoreFoundation', 'OpenGL']:
        env.Append(LINKFLAGS = [ '-framework', f ])
    return True


def prepare_for_compiling_with_glfw(env, glfw_path_override=None):
    """Find the GLFW library and add the necessary compiler/linker flags to env.
    Return True if successful, False otherwise.
    TODO: glfw_path_override currently ignored on Linux.
    """

    print("Looking for GLFW...")

    if sys.platform == "darwin":
        success = __prepare_for_compilation_mac(env, glfw_path_override)
    else:
        print(f"{env['INDENT']}GLFW not yet supported on this platform: {sys.platform}")
        return False

    if not success:
        print(f"{env['INDENT']}{NO_GLFW_MSG}")
        return False

    # TODO: put back
    # if sys.platform == 'win32':
    #     viewer_env.Append(CPPFLAGS='-D_GLFW_WIN32')
    # else:
    #     viewer_env.Append(CPPFLAGS='-D_GLFW_X11')

    print(f"{env['INDENT']}GLFW found")
    return True
