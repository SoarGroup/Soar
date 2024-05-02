# `soar-compat`

A small shim library that provides compatibility with packaging/distribution methods for SOAR's SML Bindings.

The raw bindings, as built in SOAR, are imported as `import Python_sml_ClientInterface`,
while the pypi package `soar-sml` imports as `soar_sml`.

While it possible to do `import soar_sml as Python_sml_ClientInterface`, this small library avoids that need,
and allows all existing scripts to import SOAR's SML Bindings in the way they're used to.
