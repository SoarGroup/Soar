# Developing `soar-sml`

Before installing development releases, it is always recommended to have an updated `pip`:

```
$ pip install --upgrade pip
```

## Instant: Installing from the latest commit

To install `soar-sml` *directly* from the latest `development` branch commit, without cloning it into a project directory,
run the following command:

```
$ pip install "git+https://github.com/SoarGroup/Soar#subdirectory=Core/ClientSMLSWIG/Python"
```

## Local: Installing from a project directory

To make `soar-sml` available for import in python, while actively developing the source code for it,
the following command is more suitable.

Assuming `Soar` has been cloned under `soar/`:
```bash
$ pip install -e soar/Core/ClientSMLSWIG/Python
```

The `-e` stands for "editable", this means pip will have python redirect the "actual" location of the `soar_sml`
package to be inside this project directory. Specifically, in this case, to the project root directory (`soar/`).

This command will install a python package directory under the project root (`soar/soar_sml/`), where it copies
build artifacts into, into a format that python expects.

During development, other than running the `pip` command again,
these files can be updated from builds with the `sml_python_dev` target:

```bash
$ scons sml_python_dev
```

Running the above command, and then re-running your python scripts,
will automatically incorporate any compiled changes.

## Remote: Installing a development version from the package index

Weekly development versions are uploaded to [`test.pypi.org/p/soar-sml`](https://test.pypi.org/p/soar-sml),
the latest of which can be installed with the following command:

```bash
$ pip install --pre -i https://test.pypi.org/simple/ soar-sml
```

## Packaging

Versioning is dynamic, and will calculate a version identifier according to the latest git tag.
- If the latest git tag is the current commit, it will version with that tag.
- If the latest git tag is from a past commit, it will increment the smallest segment (e.g. `.2` in `9.6.2`) and
  add miscellaneous metadata (such as the commit datetime, the amount of commits since the last tag, and wether the
  workspace repository was "dirty" or not: if there were uncommitted changes.).

`build.yml` will automatically build wheels for `soar-sml` on every commit, utilizing
[cibuildwheel](https://cibuildwheel.pypa.io/) to ease the process, and build for many python versions at once. It will
do a quick test on every build, importing `soar_sml` and running hello world, to ensure that the wheel passes basic
checks, and "works".

`build.yml` also contains job definitions to upload to PyPI, the Python Package Index.

On a GitHub release, a workflow is triggered to build the final version of wheels, and mark them
**with the version given in the corresponding git tag**.
(So if the release is named "Version 9.6.2", and the tag is "v9.6.2", then the auto-version script will see "v9.6.2")
The string `releases/` is also removed from the git tag before parsing.

*Do not release multiple versions pointing to the same commit*. Git tags do not have ordering compatible
with the dynamic versioning, and so with the tags `9.6.2-rc2` and `9.6.2` pointing to the same commit,
it may pick up on the `rc` tag, and version the build with that.

Development versions are built and uploaded to [`test.pypi.org/p/soar-sml`](https://test.pypi.org/p/soar-sml) weekly.
These will also upload on manual triggers.

If needed, uploading to pypi can be done manually using the
[twine CLI tool](https://twine.readthedocs.io/en/stable/#using-twine).
