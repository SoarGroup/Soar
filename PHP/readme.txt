=================================
PHP SWIG Bindings for Soar
Author: Nate Derbinsky
=================================

For the PHP bindings to build correctly, SWIG Version 1.3.40 is required.

For Soar to interact with PHP, there are a couple manual steps:

1. Open php.ini and set the enable_dl = On
2. Copy (or, preferably, create a symbolic link) of libPHP_sml_ClientInterface.so (in lib) to the PHP extension_dir (sans the lib prefix).
   This is visible via phpInfo() (search for extension_dir) or `php-config --extension-dir`.


For Soar to work with Apache via PHP, there are another couple steps:

1. The module needs to be loaded by default.
   Open php.ini, add extension=PHP_sml_ClientInterface.so (at the end of the list of extensions).

2. The SML shared library (i.e. libSoarKernelSML) needs to be accessible to Apache.
   Easiest way: copy the library to system library path (like /usr/local/lib).


=================================
Current Progress
=================================
This work is currently a proof-of-concept...
- Basic SWIG interface file
- OK callback scheme (uses agent name as string instead of pointer)
- A couple implemented events

In experimental use of in-the-head agents, Soar is very fast (comparable with C++), with small 
memory overhead (~10MB vs. ~60MB for Java, Mac OS X).
