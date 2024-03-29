from dataclasses import dataclass
from pathlib import Path
import os
import subprocess
import sys
from typing import Optional

NO_TCL_MSG = "Tcl cannot be built: no Tcl found"


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


def __get_tcl_from_local_dir_mac(env, local_compiled_dir=None, tcl_suffix=None) -> Optional[TclInstallInfo]:
    if not local_compiled_dir:
        return None

    if tcl_suffix is None:
        tcl_suffix = ""

    home_dir = Path(local_compiled_dir)
    install_info = TclInstallInfo(
        home=home_dir,
        lib_dir=home_dir / "lib",
        include_dir=home_dir / "include",
        dyn_lib_name=f"libtcl8.6{tcl_suffix}.dylib",
        include_lib_name=f"tcl8.6{tcl_suffix}",
    )
    valid, msg = install_info.is_valid()
    if not valid:
        print(f"{env['INDENT']}Tcl not found in directory {local_compiled_dir}: {msg}")
        return None

    return install_info

def __get_brew_tcl_install_info_mac(env) -> Optional[TclInstallInfo]:
    try:
        brew_installed_dir = (
            subprocess.check_output(["brew", "--prefix", "tcl-tk"]).decode().strip()
        )
    except subprocess.CalledProcessError:
        print(f"{env['INDENT']}Tcl not brew-installed: {msg}")
        return None

    home_dir = Path(brew_installed_dir)
    install_info = TclInstallInfo(
        home=home_dir,
        lib_dir=home_dir / "lib",
        include_dir=home_dir / "include" / "tcl-tk",
        dyn_lib_name="libtcl8.6.dylib",
        include_lib_name="tcl8.6",
    )
    valid, msg = install_info.is_valid()
    if not valid:
        print(f"{env['INDENT']}Brew-installed Tcl could not be loaded: {msg}")
        return None

    return install_info


def __get_system_tcl_install_info_mac(env) -> Optional[TclInstallInfo]:
    tcl_home = Path("/Library/Frameworks/Tcl.framework/Versions/Current")
    install_info = TclInstallInfo(
        home=tcl_home,
        lib_dir=tcl_home,
        include_dir=tcl_home / "Headers",
        dyn_lib_name="Tcl",
        include_lib_name="Tcl",
        using_framework=True,
    )
    valid, msg = install_info.is_valid()
    if not valid:
        print(f"{env['INDENT']}System Tcl not found: {msg}")
        return None
    return install_info


def __prepare_for_compiling_with_tcl_mac(env, tcl_path_override, tcl_suffix):
    install_info = __get_tcl_from_local_dir_mac(env, tcl_path_override, tcl_suffix) or \
        __get_brew_tcl_install_info_mac(env) or \
        __get_system_tcl_install_info_mac(env)
    if not install_info:
        return False

    print(f"{env['INDENT']}Found Tcl: " + str(install_info.home))

    __append_tcl_compile_flags(env, install_info)
    if install_info.using_framework:
        env.Append(LINKFLAGS=["-framework", install_info.dyn_lib_name])
    else:
        env.Append(LIBS=[install_info.dyn_lib_name])

    # Link error occurs if we include the -bundle flag with -flat_namespace, so we removed it
    env.Append(SHLINKFLAGS=env.Split('$LINKFLAGS -flat_namespace -undefined suppress -fmessage-length=0'))

    return True


def __prepare_for_compiling_with_tcl_linux(env, tcl_path_override, tcl_suffix):
    indent = env["INDENT"]

    if tcl_suffix is None:
        tcl_suffix = "t"

    if tcl_path_override:
        home_dir = Path(tcl_path_override)
        install_info = TclInstallInfo(
            home=home_dir,
            lib_dir=home_dir / "lib",
            include_dir=home_dir / "include",
            dyn_lib_name=f"tcl86{tcl_suffix}.so",
            include_lib_name=f"tcl86{tcl_suffix}",
        )
        valid, msg = install_info.is_valid()
        if not valid:
            print(f"{indent}Tcl not found in {home_dir}: {msg}")
            return False
        print(f"{indent}Found Tcl at specified location: " + str(install_info.home))
        __append_tcl_compile_flags(env, install_info)
    else:
        try:
            env.ParseConfig("pkg-config tcl --libs --cflags")
        except OSError:
            print(
                f"{indent}pkg-config didn't find tcl package; try `apt-get install tcl-dev`"
            )
            return False
        print(f"{indent}Found Tcl with pkg-config")

    return True


def __prepare_for_compiling_with_tcl_windows(env, tcl_path_override, tcl_suffix):
    indent = env["INDENT"]

    if tcl_suffix is None:
        tcl_suffix = "t"

    if tcl_path_override:
        home_dir = Path(tcl_path_override)
    else:
        home_dir = Path("C:/ActiveTcl")

    install_info = TclInstallInfo(
        home=home_dir,
        lib_dir=home_dir / "lib",
        include_dir=home_dir / "include",
        dyn_lib_name=f"tcl86{tcl_suffix}.lib",
        include_lib_name=f"tcl86{tcl_suffix}",
    )
    valid, msg = install_info.is_valid()
    if not valid:
        print(f"{indent}Tcl not found in {home_dir}: {msg}")
        print(f"{indent}{NO_TCL_MSG}")
        return False
    print(f"{indent}Found Tcl: " + str(install_info.home))

    __append_tcl_compile_flags(env, install_info)
    # Windows DLLs need to get linked to dependencies, whereas Linux and Mac shared objects do not
    # (not sure if this is really needed for TclSoarLib)
    env.Append(LIBS=[install_info.include_lib_name])

    return True


def __append_tcl_compile_flags(env, install_info: TclInstallInfo):
    env.Append(CXXFLAGS=["-I" + str(install_info.include_dir.absolute())])
    env.Append(CPPPATH=[str(install_info.include_dir.absolute())])
    env.Append(LIBPATH=[install_info.lib_dir.absolute()])


# TODO: a more ideal solution would figure out the suffix for you by examining the files in
# the Tcl directory. That's a lot of work for not many users right now; plus, Tcl 8.7 won't
# even have a suffix anymore.
def prepare_for_compiling_with_tcl(env, tcl_path_override=None, tcl_suffix=None):
    """Find the Tcl library and add the necessary compiler/linker flags to env.
    Return True if successful, False otherwise.
    tcl_path_override: If specified, this path will be searched first for a Tcl installation.
    tcl_suffix: If specified, the suffix will be expected on Tcl binary names. Tcl 8.6 uses
    "t" on Linux and Windows and "" on Mac by default. The suffix is supposed to indicate the
    type of the Tcl build, but can be overridden by the user when building Tcl.
    """

    indent = env["INDENT"]

    print("Looking for Tcl...")
    if tcl_path_override:
        print(f"{indent}Tcl path override specified: {tcl_path_override}")
        if not Path(tcl_path_override).exists():
            raise FileNotFoundError(
                f"Specified Tcl path does not exist: {tcl_path_override}"
            )

    if sys.platform == "darwin":
        if not __prepare_for_compiling_with_tcl_mac(env, tcl_path_override, tcl_suffix):
            print(f"{indent}{NO_TCL_MSG}")
            return False

    elif sys.platform.startswith("linux"):
        if not __prepare_for_compiling_with_tcl_linux(env, tcl_path_override, tcl_suffix):
            print(f"{indent}{NO_TCL_MSG}")
            return False

    elif sys.platform == "win32":
        if not __prepare_for_compiling_with_tcl_windows(env, tcl_path_override, tcl_suffix):
            print(f"{indent}{NO_TCL_MSG}")
            return False

    if os.name == "posix":
        # -fPic is needed to make the code position independent, which is necessary for Tcl.
        env.Append(CPPFLAGS=["-fPIC"])

    return True
