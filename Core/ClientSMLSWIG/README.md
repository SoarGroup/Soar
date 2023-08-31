# Client SML SWIG Bindings

[SWIG](https://www.swig.org/) is a tool for connecting C/C++ libraries to to other languages. We currently use it to generate bindings for the SML client library to Java, Python, C#, and Tcl.

## Files

* `*.i`` files contain directives for SWIG. `sml_ClientInterface.i` in the top-level directory contains directives common to each target language.
* `*CallbackByHand.h` files contain manually-written C wrappers related to callback functions. These could not be generated automatically by SWIG (TODO: why?).

## New SML Members

If new functions or classes are added to the `sml` namespace, SWIG will try to export them automatically. If you want to prevent this, you need to add a `%ignore` directive to `sml_ClientInterface.i`. You will also need to do this for any functions that require a custom implementation for a target language.

## Portability

SWIG generates pure C wrappers for the SML client library, which are then compiled into shared libraries for each target language. SWIG also generates idiomatic code in each target language for loading and using the library. Because the library is compiled from C, it is much more portable across platforms, and the symbol names are not mangled.

In order to guarantee C-style symbol linking (e.g. no mangling), we use `extern "C"`. To maintain our portability goals, it is very important to heed any compiler warnings related to `extern "C"` incompatibilities, as they indicate an API that is not being portably exported. Note that `std::string`s are okay to use because we load SWIG's typemaps for converting them.

### Python

Python bindings will work only with the major version that they are built against. So if we build against 3.11.1, it will work with 3.11.2, etc., but not 3.10 or 3.12.

### Java

Java bindings should theoretically work with any major version equal to or higher than what we build with.

### C#

TODO

### Tcl

TODO
