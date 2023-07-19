# Soar

[![Build/Test](https://github.com/SoarGroup/Soar/actions/workflows/build.yml/badge.svg)](https://github.com/SoarGroup/Soar/actions/workflows/build.yml)

This is the code base for Soar, a cognitive architecture for developing systems that exhibit intelligent behavior. For more information about this project, please visit:

* [The Soar home page](http://soar.eecs.umich.edu/)
* [The GitHub project page](https://github.com/SoarGroup/Soar)

Note that the readme included with the Soar distribution for endusers is in the [Release-Support](https://github.com/SoarGroup/Release-Support/blob/master/txt/README) repository.

# Soar Builds

For binary builds of Soar you can get them in two places:

* [Official Releases](http://soar.eecs.umich.edu/articles/downloads/soar-suite)
* [Latest Successful Development Build](https://github.com/SoarGroup/Soar/actions/workflows/build.yml?query=branch%3Adevelopment): click the latest run and scroll down to "Artifacts".
  - Note: if the download for your platform isn't there, the build failed. You'll need to download the result of an earlier build.

# Soar Performance

Some performance statistics are calculated automatically using the Factorization Stress Tests.  You can see performance on a commit-by-commit basis either in [Performance.md](https://github.com/SoarGroup/Soar/blob/development/Performance.md) or [here](http://soar-jenkins.eecs.umich.edu/Performance/). The raw data used to generate the graphs for each build can be found [here](http://soar-jenkins.eecs.umich.edu/Performance/).

Disclaimer: These are worst case tests.  Average performance is probably much higher.  In addition, these show that even in worst case, Soar beats its goal of 50 msec reactivity (in these tests, the max is ~30msec per decision).

# Development

## Prerequisites

To compile Soar, you will need the following:

* C/C++ compiler
    - Mac: `xcode-select --install`
    - Linux: `sudo apt-get install build-essential`
* Python
    - Mac: `brew install python`
* Java
    - We recommend using [SDKMan](https://sdkman.io/). Install JDK 17 (Temurin is recommended).

To compile the extra SML wrapper libs, you will need the following:

* C# compiler (`csc`)
    - Mac: `brew install mono`
* Tcl
    - Mac: `brew install tcl-tk`

The project supports generating compile_commands.json, which can be used by e.g. VSCode with the C/C++ plugin to provide IntelliSense. To generate this file, run scons with the `cdb` target:

   python3 scons/scons.py --scu --opt --verbose cdb

# License

Soar is available under the following [LICENSE](https://github.com/SoarGroup/Soar/blob/development/LICENSE.md).  This license is [BSD](http://opensource.org/licenses/BSD-2-Clause)
