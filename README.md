# Soar

[![CMake Multi-Platform](https://github.com/SoarGroup/Soar/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/SoarGroup/Soar/actions/workflows/cmake-multi-platform.yml)
[![Build/Test](https://github.com/SoarGroup/Soar/actions/workflows/build.yml/badge.svg)](https://github.com/SoarGroup/Soar/actions/workflows/build.yml)

This is the code base for Soar, a cognitive architecture for developing systems
that exhibit intelligent behavior. For more information about this project,
please visit:

- [The Soar home page](http://soar.eecs.umich.edu/)
- [The GitHub project page](https://github.com/SoarGroup/Soar)

Note that the readme included with the Soar distribution for end-users is in the
[Release-Support](https://github.com/SoarGroup/Release-Support/blob/master/txt/README.md)
repository.

## Soar Builds

For binary builds of Soar you can get them in two places:

- [Official Releases](http://soar.eecs.umich.edu/articles/downloads/soar-suite)
  — multiplatform zip including `bin/win_x86-64`, `bin/linux_x86-64`,
  `bin/mac_x86-64`, and `bin/mac_ARM64` (Apple Silicon).
- Latest development builds: the CMake-based CI pipeline
  ([`cmake-multi-platform.yml`](https://github.com/SoarGroup/Soar/actions/workflows/cmake-multi-platform.yml?query=branch%3Adevelopment))
  and multi-platform release workflow
  ([`release.yml`](https://github.com/SoarGroup/Soar/actions/workflows/release.yml))
  produce per-platform artifacts including ARM64 and universal (`arm64` + `x86_64`) macOS builds.
  The SCons CI
  ([`build.yml`](https://github.com/SoarGroup/Soar/actions/workflows/build.yml?query=branch%3Adevelopment))
  is also kept current and builds the `soar-sml` Python wheel packages across platforms.

## Soar Performance

Some performance statistics are calculated automatically using the
Factorization Stress Tests. You can see performance on a commit-by-commit basis
in [Performance.md](https://github.com/SoarGroup/Soar/blob/development/Performance.md).

Disclaimer: These are worst case tests. Average performance is probably much
higher. In addition, these show that even in worst case, Soar beats its goal of
50 msec reactivity (in these tests, the max is ~30msec per decision).

## Development

Soar supports two build systems: `CMake` (see [Build with CMake](#build-with-cmake))
and `SCons` (see [Build with SCons](#build-with-scons)). Both are kept in sync
and produce binary-compatible artifacts that land in the same `out/` directory and
multi-platform install layout. Official multi-platform `SoarSuite` release archives
are built and assembled by [`release.yml`](.github/workflows/release.yml) and
[`scripts/assemble-release.cmake`](scripts/assemble-release.cmake).

The following table compares supported build features for Soar between the two
build systems:

| Feature                           | SCons | CMake |
| --------------------------------- | ----- | ----- |
| Soar dynamic lib (`Soar.dll` / `libSoar`) | ✅ | ✅ |
| Soar static lib (`--static` / `BUILD_SHARED_LIBS=OFF`) | ✅ | ✅ |
| Soar CLI (`soar`)                 | ✅    | ✅    |
| Unit tests (`UnitTests`)          | ✅    | ✅    |
| Performance tests (`PerformanceTests`) | ✅ | ✅  |
| External lib test (`TestExternalLibraryLib`) | ✅ | ✅ |
| SVS (`SVS=ON` / default in SCons) | ✅    | ✅    |
| SWIG Python (`Py_LIMITED_API` / ABI3) | ✅ | ✅   |
| SWIG Java (`sml.jar` + JNI)       | ✅    | ✅    |
| SWIG JavaScript (Node-API `.node`) | ❌   | ✅    |
| SWIG C# (`sml_csharp.dll` + P/Invoke) | ✅ | ✅  |
| SWIG Tcl (`Tcl_sml_ClientInterface`) | ✅ | ✅   |
| Embedded Tcl (`tclsoarlib`)       | ✅    | ✅    |
| Java Debugger (`SoarJavaDebugger.jar`) | ✅ | ✅  |
| Python wheel package (`soar-sml`) | ✅    | ❌    |
| Generate `compile_commands.json`  | ✅    | ✅    |
| Release & Debug configurations    | ✅    | ✅    |
| Debug with AddressSanitizer       | ❌    | ✅    |
| Conan package manager integration | ❌    | ✅    |
| Multi-platform release assembler  | ❌    | ✅    |
| macOS (`arm64`, `x86_64`, fat universal) | ✅ | ✅ |
| Linux (`x86_64`)                  | ✅    | ✅    |
| Windows (`x86_64`, VS 2022 / 2026) | ✅   | ✅    |

### Prerequisites

The most up-to-date reference for compiling Soar from source is the CI workflows:
- [`cmake-multi-platform.yml`](.github/workflows/cmake-multi-platform.yml) (CMake PR/branch CI across presets, SVS, and SWIG)
- [`release.yml`](.github/workflows/release.yml) (Full CMake multi-platform `SoarSuite` release assembly)
- [`build.yml`](.github/workflows/build.yml) (SCons build, language binding tests, and `cibuildwheel` Python packaging)

Common dependencies across both build systems:

- **C/C++17 compiler**
  - Windows: Visual Studio 2022 or 2026 (Desktop development with C++)
  - macOS: `xcode-select --install`
  - Linux: `sudo apt-get install build-essential`
- **Python 3** (3.8+)
  - macOS: `brew install python`
- **Java JDK 11+** (required for `sml_java` / `SWIG_JAVA` and `SoarJavaDebugger`)
  - [SDKMan](https://sdkman.io/) or Eclipse Temurin JDK 11+ is recommended.

Optional dependencies for SML language bindings and Tcl:

- **SWIG** (4.2+ recommended for Python Stable ABI / `Py_LIMITED_API`)
  - Windows: `choco install swig` or `pip install swig`
  - macOS: `brew install swig`
  - Linux: `pip install swig` (or `sudo apt install swig` on Ubuntu 24.04+)
- **Tcl 8.6** (needed for `SWIG_TCL` / `sml_tcl` and `TCLSOARLIB` / `tclsoarlib`)
  - macOS: `brew install tcl-tk@8`
  - Linux: `sudo apt install tcl-dev pkgconf`
  - Windows: Tcl 8.6 installation (e.g. `C:\ActiveTcl` or built from source in `C:\Tcl`)
- **.NET SDK 6.0+** (needed for `SWIG_CSHARP` / `sml_csharp`)
  - `dotnet` CLI (`brew install --cask dotnet-sdk` / `winget install Microsoft.DotNet.SDK.8`)
- **Node.js + `node-addon-api`** (needed for CMake `SWIG_JAVASCRIPT`)
  - `npm install -g node-addon-api`

### Build with SCons

SCons uses the bundled `scons/scons.py` (with `Core/SoarKernel/sqlite` and `Core/SVS/eigen` included in-tree, so Conan is not required). On Windows, run from a **x64 Native Tools Command Prompt for VS** or use `build.bat` (which caches detected tool paths in `user-env.bat`).

#### Common SCons Commands

```shell
# Build default targets (kernel, cli, sml_java, debugger, headers, scripts, cdb) in Release mode:
python3 scons/scons.py --scu --opt

# Build all targets (including UnitTests, SWIG bindings, and TclSoarLib) in Debug mode (assertions enabled):
python3 scons/scons.py --scu --dbg --verbose all

# Build all targets + performance tests + Tcl SML bindings in Release mode (Windows wrapper):
.\build.bat --scu --opt --tcl="C:\ActiveTcl" all performance_tests sml_tcl

# Generate only compile_commands.json for IDE IntelliSense:
python3 scons/scons.py --scu --opt cdb
```

#### SCons Targets and Options

- **Core & App targets**: `kernel`, `cli`, `debugger`, `tclsoarlib`, `headers`, `scripts`, `cdb`, `all`
- **Testing targets**: `tests` (builds `out/UnitTests`, `out/TestExternalLibraryLib`, and copies `out/SoarUnitTests`), `performance_tests`
- **SWIG targets**: `sml_python`, `sml_java`, `sml_tcl`, `sml_csharp`
- **Key flags**:
  - `--opt` (default, optimized Release) or `--dbg` (Debug symbols + kernel assertions)
  - `--scu` (default, Single Compilation Unit unity build for fast compilation) or `--no-scu`
  - `--no-svs` (disable SVS spatial/visual system)
  - `--static` (build static library instead of shared library)
  - `--tcl=<path>` and `--tcl-suffix=<suffix>` (specify Tcl root directory and library suffix)
  - `--build=<dir>` and `--out=<dir>` (customize intermediate build directory and output directory)
  - Set environment variable `SOAR_PYTHON_ABI3=1` (with SWIG 4.2+) to compile Python bindings against the Python 3.8+ Limited API (`Py_LIMITED_API`).

Note for Apple Silicon Mac users: ensure your Python interpreter and `clang` are running natively on `arm64` (not under Rosetta `x86_64` translation) by checking `python3 -c "import platform; print(platform.machine())"`.

### Build with CMake

CMake builds place runtime and library outputs into `out/` (matching the SCons layout so `SOAR_HOME` and tools work identically) and support `cmake --install` for staged distribution trees.

#### Prerequisites for CMake

- **CMake >= 3.23** (`>= 3.26` when `SWIG_PYTHON=ON` for Python `Development.SABIModule` support)
- **Conan 2** and **Ninja**: `pip install conan ninja`
  - Run `conan profile detect` once if you do not have a default Conan profile.

#### Path 1: Preset-Based Workflow (Recommended)

First, install Conan dependencies (`sqlite3`, `asio`, `eigen`) for the build configuration(s) you plan to use:

```shell
# Unix / macOS:
conan install . --build=missing -s build_type=Release
conan install . --build=missing -s build_type=Debug

# Windows (use Ninja generator in a Visual Studio x64 Developer prompt):
conan install . --build=missing -s build_type=Release -c tools.cmake.cmaketoolchain:generator=Ninja
conan install . --build=missing -s build_type=Debug -c tools.cmake.cmaketoolchain:generator=Ninja
```

Then choose a preset from [`CMakePresets.json`](./CMakePresets.json):

| Preset             | What it builds                                                                 |
| ------------------ | ------------------------------------------------------------------------------ |
| `Release-test`     | Release static `soar_lib` + `soar` CLI + `UnitTests` + `PerformanceTests`      |
| `Release-shared`   | Release shared `Soar` library + `soar` CLI + `UnitTests`                       |
| `Release-svs-test` | Release with SVS enabled + `soar` CLI + `UnitTests` (including `test_svs`)     |
| `Release-swig`     | Release shared `Soar` + SVS + CLI + `SoarJavaDebugger` + Python/Tcl/Java/C# SWIG + `tclsoarlib` |
| `Debug-test`       | Debug static `soar_lib` + `soar` CLI + `UnitTests` (assertions on)             |
| `Debug-shared`     | Debug shared `Soar` library + `soar` CLI + `UnitTests`                         |
| `Debug-test-asan`  | Debug + `UnitTests` + AddressSanitizer (Linux/macOS)                           |
| `Debug-swig-full`  | Debug shared `Soar` + SVS + CLI + `SoarJavaDebugger` + all SWIG bindings       |

Configure, build, and run the CTest suite:

```shell
cmake --preset Release-svs-test
cmake --build --preset Release-svs-test
ctest --preset Release-svs-test
```

Or run the configure + build + test workflow in a single command:

```shell
cmake --workflow --preset Release-test-workflow
cmake --workflow --preset Release-svs-test-workflow
```

#### Path 2: Custom Feature Flags & Language Bindings

You can override any CMake option on top of a preset (or pass `-DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake` directly):

```shell
# Example: Release-swig with custom Tcl path on macOS or Windows
cmake --preset Release-swig -DCMAKE_PREFIX_PATH="C:/Tcl"
cmake --build --preset Release-swig
cmake --install build/Release --prefix install
```

Available CMake cache options (all `OFF` by default when consuming Soar via `FetchContent`, enabled by presets):
- `-DBUILD_SHARED_LIBS=ON|OFF` — Build shared (`Soar.dll` / `libSoar.so` / `libSoar.dylib`) vs. static library
- `-DSVS=ON|OFF` — Enable the Spatial Visual System (`ccd`, `Eigen3`, `asio`)
- `-DCLI=ON|OFF` — Build the `soar` command-line interface executable
- `-DBUILD_TESTS=ON|OFF` — Build `UnitTests`, `TestExternalLibraryLib`, and `PerformanceTests` and register CTest targets (`test_ci`, `test_svs`)
- `-DSWIG=ON|OFF` — Enable all SWIG bindings (`SWIG_PYTHON`, `SWIG_JAVA`, `SWIG_TCL`, `SWIG_CSHARP`, `SWIG_JAVASCRIPT`) or toggle each individually
- `-DJAVA_DEBUGGER=ON|OFF` — Build `SoarJavaDebugger.jar` (requires `-DSWIG_JAVA=ON`)
- `-DTCLSOARLIB=ON|OFF` — Build `tclsoarlib` embedded Tcl interpreter library

#### Path 3: Multi-Platform Release Assembly

To assemble a complete `SoarSuite_<version>-Multiplatform.zip` bundle from per-platform `cmake --install` trees (`platforms/soar-{win_x86-64,linux_x86-64,mac_x86-64,mac_ARM64}`) and auxiliary repositories (`Agents`, `Release-Support`, `VisualSoar`):

```shell
cmake -P scripts/assemble-release.cmake -- <version> <platforms-dir> [repos-dir]
```

`compile_commands.json` is generated automatically in CMake builds and picked up by VS Code's C/C++ extension and `clangd`. See [`CMakePresets.json`](./CMakePresets.json) for all preset definitions.

## License

Soar is available under the following
[LICENSE](https://github.com/SoarGroup/Soar/blob/development/LICENSE.md). This
license is [BSD](http://opensource.org/licenses/BSD-2-Clause)
