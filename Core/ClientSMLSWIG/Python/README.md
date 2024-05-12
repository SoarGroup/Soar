# `soar-sml`

This directory contains the SWIG bindings to Soar's SML interface.

This project is also published on PyPI as [soar-sml](https://pypi.org/project/soar-sml/),
for ease of distribution and installation.

Versions on PyPI will follow Soar's versioning methodology.

`soar-sml` effectively bundles Soar with its distribution; you can download it, import it into Python,
and have a fully-working Soar kernel up and running.

## Importing

The soar-sml package can be imported like so:

```Python
import soar_sml
```

## `Python_sml_ClientInterface` compatibility

The raw build artifacts of this SWIG interface exposed these bindings under a `Python_sml_ClientInterface` namespace
in the past, we've switched to using `soar_sml` to be more in line with python's packaging ideology. However,
we did not want to break compatibility with all scripts by doing so, and have also published a compatibility shim.

Under `compat/`, there exists a small project `soar-compat` that re-exports the classes and functions of under a
`Python_sml_ClientInterface` namespace, effectively making every existing project compatible with the new `soar-sml`
package.

This means that code like thus will still continue to work:

```Python
import sys
sys.path.append('/home/user/SoarSuite/bin')
import Python_sml_ClientInterface as sml

k = sml.Kernel.CreateKernelInNewThread()
a = k.CreateAgent('soar')
print(a.ExecuteCommandLine('echo hello world'))
```

In this case, the `sys.path.append` line is redundant (it accomplishes nothing; python already properly imports
the package), and can be removed.

This compatibility package is available on PyPI, and can be installed directly like so:

```bash
$ pip install soar-compat --no-deps
```
(the `--no-deps` flag is added to prevent `soar-sml` being pulled from pypi, as that is a dependency of
`soar-compat`)

However, for ease of installation, it can be installed as an ["extra"](https://stackoverflow.com/a/52475030/8700553)
like so:

```bash
$ pip install "soar-sml[compat]"
```

Running this above command will make every Soar SML Python script on your system (or virtual environment) that uses
`import Python_sml_ClientInterface` functional, portable, with no further modifications necessary.

## Building locally

Building and installing this package via pip, locally, is easy:

```BASH
$ pip install soar/Core/ClientSMLSWIG/Python
```

This will generate all the required build artifacts, install them to your python installation (system-wide, or
inside a virtual environment), and prepare it for `import soar_sml` statements in your scripts/programs.

## Developing and Packaging

Notes for developing and packaging `soar-sml` can be found in [`DEVELOPING.md`](./DEVELOPING.md)
