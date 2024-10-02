# Soar

[![Build/Test](https://github.com/SoarGroup/Soar/actions/workflows/build.yml/badge.svg)](https://github.com/SoarGroup/Soar/actions/workflows/build.yml)

This is the code base for Soar, a cognitive architecture for developing systems that exhibit intelligent behavior. For more information about this project, please visit:

* [The Soar home page](http://soar.eecs.umich.edu/)
* [The GitHub project page](https://github.com/SoarGroup/Soar)

Note that the readme included with the Soar distribution for end-users is in the [Release-Support](https://github.com/SoarGroup/Release-Support/blob/master/txt/README) repository.

## Soar Builds

For binary builds of Soar you can get them in two places:

* [Official Releases](http://soar.eecs.umich.edu/articles/downloads/soar-suite)
* [Latest Successful Development Build](https://github.com/SoarGroup/Soar/actions/workflows/build.yml?query=branch%3Adevelopment): click the latest run and scroll down to "Artifacts".
    * If the download for your platform isn't there, the build failed. You'll need to download the result of an earlier build.
    * GitHub cannot build for ARM64 (M-series Macs), so you'll need to build from source or use the release version instead.

## Soar Performance

Some performance statistics are calculated automatically using the Factorization Stress Tests.  You can see performance on a commit-by-commit basis either in [Performance.md](https://github.com/SoarGroup/Soar/blob/development/Performance.md) or [here](http://soar-jenkins.eecs.umich.edu/Performance/). The raw data used to generate the graphs for each build can be found [here](http://soar-jenkins.eecs.umich.edu/Performance/).

Disclaimer: These are worst case tests.  Average performance is probably much higher.  In addition, these show that even in worst case, Soar beats its goal of 50 msec reactivity (in these tests, the max is ~30msec per decision).

## Development

### Prerequisites

The instructions below are cursory and may be out of date; the most up-to-date instructions for compiling Soar from source will always be the CI build scripts. You can find them [here](.github/workflows/build.yml).

To compile Soar, you will need the dependencies listed below. Note that the installation commands are not complete, e.g. missing instructions for Mac do not mean that the dependency is not needed on Mac, etc.:

* C/C++ compiler
    * Mac: `xcode-select --install`
    * Linux: `sudo apt-get install build-essential`
* Python
    * Mac: `brew install python`
* Java
    * We recommend using [SDKMan](https://sdkman.io/). The debugger, etc. require Java 11 at a minimum, but it's best to install the latest LTS. Temurin is recommended.

To compile the extra SML wrapper libs, you will need the following:

* pkg-config
    * Mac: `brew install pkg-config`
    * Linux: `sudo apt install pkgconf`
* SWIG
    * Mac: `brew install swig`
    * Linux: `sudo apt install swig`
* Python development headers (only needed for Python wrapper)
    * Linux: `sudo apt install python3-dev`
* C# compiler (`csc`) (only needed for C# wrapper)
    * Mac: `brew install mono`
* Tcl (only needed for Tcl wrapper and TclSoarlib)
    * Mac: `brew install tcl-tk`

The project supports generating compile_commands.json, which can be used by e.g. VSCode with the C/C++ plugin to provide IntelliSense. To generate this file, run scons with the `cdb` target:

   python3 scons/scons.py --scu --opt --verbose cdb

Note for M-series Mac users: you'll want to make sure you're compiling for ARM64, not x86_64. Sometimes users have Python installed in compatibility mode, leading to compiles for the wrong architecture. You can check which architecture your Python is built for using this:

```python
import sysconfig
>>> print(sysconfig.get_config_vars())
```

You can also check your `clang`'s default compile target using `clang --version`.

To compile everything for local development, you can use the following command:

```shell
   python3 scons/scons.py --scu --dbg --verbose all
```

Debug mode enables debugging, but also activates assertions, which are important for catching bugs early. `--scu` (single compilation unit) simplifies the debugging experience.

If you want an optimized build instead:

```shell
    python3 scons/scons.py --opt --verbose all
```

## License

Soar is available under the following [LICENSE](https://github.com/SoarGroup/Soar/blob/development/LICENSE.md).  This license is [BSD](http://opensource.org/licenses/BSD-2-Clause)
