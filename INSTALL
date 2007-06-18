Soar Suite version 8.6.2


To install Soar Suite, a choice needs to be made whether to install it
system-wide or on a per-user basis. Due to the nature of the Soar Suite
package, we recommend installing it on a per-user basis.

Installing on a per-user basis
==============================

To install Soar Suite on a per user basis, the path to the SoarLibrary
directory must be used as the --prefix switch. For example, assuming you
downloaded the Soar Suite package to your home directory:
	
	/home/username/soarsuite-8.6.2.tar.gz
	
Extracting the package at this location yields the soarsuite-8.6.2
directory (the location of this file):

	/home/username/soarsuite-8.6.2
	/home/username/soarsuite-8.6.2/INSTALL
	etc.
   
To configure Soar, you must use the --prefix switch, pointing it to the
SoarLibrary directory:

	(in soarsuite-8.6.2):
	./configure --prefix=/home/username/soarsuite-8.6.2/SoarLibrary

More information on the configure step is included below. A common
failure is the inability for the system to find the java include file,
jni.h. You may point it to the location of your java installation using
the following switch (along with the --prefix switch) on the configure
command line:

	--with-javaincl=/path/to/java/include

Note that you provide the path to the "include" directory, not to the
jni.h file itself. If you want to disable java (not recommended, many of
the more useful Soar applications use Java), use the following switch
instead:

	--disable-java

Once configured, build and install the project using the following
commands:

	(in soarsuite-8.6.2):
	make
	make install

Build the java applications next using the following command:

	(in soarsuite-8.6.2):
	make java

Finally, the SoarLibrary/bin folder needs to be added to the path, and
the SoarLibrary/lib folder needs to be added to the library path. To do
this, the environment variables PATH and LD_LIBRARY_PATH need to be
modified. The best way to do this is to modify your shell initialization
script to do this for you when you log in, this way the changes you make
will persist throughout many sessions. Detail of this step vary from
shell to shell. The following is an example how to do this in the very
common bash shell:

	(add to /home/username/.bashrc):
	PREFIX=/home/username/soarsuite-8.6.2/SoarLibrary
	PATH=${PREFIX}/bin:${PATH}
	LD_LIBRARY_PATH=${PREFIX}/lib:${LD_LIBRARY_PATH}

To apply these changes immediatly, source the new shell script:

	source ~/.bashrc

You can view these variables to check they are correct:

	echo $PREFIX
	echo $PATH
	echo $LD_LIBRARY_PATH

If you have any problems with this installation process, please first
consult the living online documentation on the Soar Wiki before sending
mail to the Soar SML development list. The Soar Wiki can be found at:

	http://winter.eecs.umich.edu/soarwiki

The Soar SML development list email address is:

	soar-sml-list@umich.edu

Installing system-wide
======================

Installing system-wide requires super-user access during the install
phase.  This form of installation is not recommended because of how Soar
applications are run (see "Running Soar Applications" below). To install
system-wide, simply run the following commands:

	./configure
	make
	(become super-user, then):
	make install
	(return to regular user, then):
	make java

Running Soar applications
=========================

Soar tools, after being compiled and installed, live in the bin
directory under the prefix. If you installed Soar using the per-user
instructions, this directory is SoarLibrary/bin, otherwise, it is likely
to be the default: /usr/local/bin

The Soar libraries are installed in the lib folder next to the bin
folder. The per-user installation instructions put the libraries in
SoarLibrary/lib

To run many Soar tools, first switch to the SoarLibrary/bin directory
before calling them. Many of the environments (such as JavaTankSoar and
JavaEaters) are run from the directory their Jar file lives in, such as
Environments/JavaTankSoar and Environments/JavaEaters.

	(to run Soar tools):
	cd SoarLibrary/bin
	TOHSML
	java -jar SoarJavaDebugger.jar

	(to run Soar environments):
	cd Environments/JavaTankSoar
	java -jar JavaTankSoar.jar
	
	cd Environments/JavaEaters
	java -jar JavaEaters.jar

More information on the configure process
=========================================

   The `configure' shell script attempts to guess correct values for
various system-dependent variables used during compilation.  It uses
those values to create a `Makefile' in each directory of the package.
It may also create one or more `.h' files containing system-dependent
definitions.  Finally, it creates a shell script `config.status' that
you can run in the future to recreate the current configuration, and a
file `config.log' containing compiler output (useful mainly for
debugging `configure').

   It can also use an optional file (typically called `config.cache'
and enabled with `--cache-file=config.cache' or simply `-C') that saves
the results of its tests to speed up reconfiguring.  (Caching is
disabled by default to prevent problems with accidental use of stale
cache files.)

   If you need to do unusual things to compile the package, please try
to figure out how `configure' could check whether to do them, and mail
diffs or instructions to soar-sml-list@umich.edu so they can be 
considered for the next release.  If you are using the cache, and at
some point `config.cache' contains results you don't want to keep, you
may remove or edit it.

   The file `configure.ac' (or `configure.in') is used to create
`configure' by a program called `autoconf'.  You only need
`configure.ac' if you want to change it or regenerate `configure' using
a newer version of `autoconf'.

