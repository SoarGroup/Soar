#!/usr/bin/python
#################
# Soar SCons build configuration file
# Author: Jonathan Voigt <voigtjr@gmail.com>
# Date: April 2007
#
# For information on SCons, see http://www.scons.org/
# For Soar-specific SCons information, see the Soar Wiki:
#   http://winter.eecs.umich.edu/soarwiki/Soar_SCons_build_information

import os
import sys
import SoarSCons

# TODO: pull this version string out of shared/soarversion.h
soarversionstring = "9.0.0"
print "Soar", soarversionstring
print "Detected OS:", os.name
print "Detected platform:", sys.platform
processor = os.popen( 'uname -p', 'r' ).read().strip()
print "Detected processor:", processor

if os.name != "posix":
	print "Unsupported OS."
	Exit(1)

#pythonDefault = 'yes'
# changed to no because it doesn't work out of the box on ubuntu
pythonDefault = 'no'
if sys.platform == 'cygwin':
	print "Disabling python by default, it will not yet build on this platform."
	pythonDefault = 'no'
	
#################
# Command line options

# OSX is identified as os.name 'posix' and sys.platform 'darwin'
# For now we're assuming other 'posix' platforms are linux
# and will deal with portability issues as they appear.

opts = Options()
opts.AddOptions(
	BoolOption('scu', 'Build using single compilation units (faster)', 'yes'), 
	BoolOption('java', 'Build the Soar Java interface (required for debugger)', 'yes'), 
	BoolOption('swt', 'Build Java SWT projects (required for debugger)', 'yes'), 
	BoolOption('python', 'Build the Soar Python interface', pythonDefault), 
	BoolOption('csharp', 'Build the Soar CSharp interface', 'no'), 
	BoolOption('tcl', 'Build the Soar Tcl interface', 'no'), 
	BoolOption('debug', 'Build with debugging symbols', 'yes'),
	BoolOption('warnings', 'Build with warnings', 'yes'),
	EnumOption('optimization', 'Build with optimization (May cause run-time errors!)', 'full', ['no','partial','full'], {}, 1),
	BoolOption('eclipse', 'Build everything except the java projects (prepare for eclipse)', 'no'),
	BoolOption('preprocessor', 'Only run preprocessor', 'no'),
	BoolOption('verbose', 'Verbose compiler output', 'no'),
	
	BoolOption('gcc42', 'Use GCC-4.2 (experimental, Darwin only)', 'no'),
	BoolOption('m64', 'Compile to 64-bit (experimental)', 'no'),
)

# Create the environment using the options
env = Environment(options = opts, ENV = os.environ)
Help(opts.GenerateHelpText(env))

#################
# Create configure context to configure the environment

custom_tests = {
	# A check to see if the -fvisibility flag works on this system.
	'CheckVisibilityFlag' : SoarSCons.CheckVisibilityFlag,
}
conf = Configure(env, custom_tests = custom_tests)

# We define SCONS to indicate to the source that SCONS is controlling the build.
conf.env.Append(CPPFLAGS = ' -DSCONS')

# We need to know if we're on darwin because malloc.h doesn't exist, functions are in stdlib.h
if sys.platform == "darwin":
	conf.env.Append(CPPFLAGS = ' -DSCONS_DARWIN')
	if conf.env['gcc42']:
		env['CXX'] = '/usr/bin/g++-4.2'
	#if conf.env['ppc']:
		#conf.env.Append(CPPFLAGS = ' -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc ')
		#conf.env.Append(LINKFLAGS = ' -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc ')		


# All C/C++ modules rely or should rely on this include path (houses portability.h)
conf.env.Append(CPPPATH = ['#Core/shared'])

# This allows us to use Java and SWIG if they are in the path.
conf.env.Append(ENV = {'PATH' : os.environ['PATH']})

# configure java if requested
if conf.env['java']:
	# This sets up the jni include files
	if not SoarSCons.ConfigureJNI(conf.env):
		print "Could not configure Java. If you know where java is on your system,"
		print "set environment variable JAVA_HOME to point to the directory containing"
		print "the Java include, bin, and lib directories."
		print "You may disable java, see help (scons -h)"
		print "Java Native Interface is required... Exiting"
		Exit(1)

	# This checks for the swt.jar and attempts to download it if it doesn't exist
	if env['swt'] and not SoarSCons.CheckForSWTJar(conf.env):
		print "Could not find swt.jar. You can obtain the jar here:"
		print "\thttp://ai.eecs.umich.edu/~soar/sitemaker/misc/jars"
		print "Place swt.jar in SoarLibrary/bin"
		print "You may disable swt, see help (scons -h)"
		print "swt.jar required... Exiting"
		Exit(1)

# check SWIG version if necessary
# SWIG is necessary if one of the swig projects is going to be built
if conf.env['java'] or conf.env['python'] or conf.env['csharp'] or conf.env['tcl']:
	if not SoarSCons.CheckSWIG(conf.env):
		explainSWIG = ""
		if conf.env['java']:
			explainSWIG += "java=1, "
		if conf.env['python']:
			explainSWIG += "python=1, "
		if conf.env['csharp']:
			explainSWIG += "csharp=1, "
		if conf.env['tcl']:
			explainSWIG += "tcl=1, "

		print "SWIG is required because", explainSWIG[:-2]
		Exit(1)
	
# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if conf.CheckVisibilityFlag():
	conf.env.Append(CPPFLAGS = ' -fvisibility=hidden')

# configure misc command line options
if conf.env['debug']:
	conf.env.Append(CPPFLAGS = ' -g3')
	#if sys.platform == 'cygwin':
	#	conf.env.Append(CPPFLAGS = ' -D_DEBUG')
if conf.env['warnings']:
	conf.env.Append(CPPFLAGS = ' -Wall')
if conf.env['optimization'] == 'partial':
	conf.env.Append(CPPFLAGS = ' -O2')
if conf.env['optimization'] == 'full':
	conf.env.Append(CPPFLAGS = ' -O3')

# Cross compile to mingw
if sys.platform == 'cygwin':
	conf.env.Tool('crossmingw', toolpath = ['.'])
	conf.env.Append(CPPFLAGS = ' -mno-cygwin')
	conf.env.Append(LINKFLAGS = ' -mno-cygwin')
	#conf.env.Append(LINKFLAGS = ' -mno-cygwin -Wl,-add-stdcall-alias')

if conf.env['preprocessor']:
	conf.env.Append(CPPFLAGS = ' -E')

if conf.env['verbose']:
	conf.env.Append(CPPFLAGS = ' -v')
	conf.env.Append(LINKFLAGS = ' -v')

# check for required libraries
if sys.platform != 'cygwin':
	if not conf.CheckLib('dl'):
		Exit(1)
		
	if not conf.CheckLib('m'):
		Exit(1)
	
	if not conf.CheckLib('pthread'):
		Exit(1)
		
# if this flag is not included, the linker will complain about not being able
# to find the symbol __sync_sub_and_fetch_4 when using g++ 4.3
# only do not include it if we're on powerpc
if processor != 'powerpc':
	if env['m64']:
		conf.env.Append(CPPFLAGS = ' -m64 -DSOAR_64 -fPIC')
		conf.env.Append(LINKFLAGS = ' -m64')
	else:
		conf.env.Append(CPPFLAGS = ' -m32')
		conf.env.Append(LINKFLAGS = ' -m32')
conf.env[ 'processor' ] = processor

env = conf.Finish()
Export('env')

#################
# Build modules

# Core
SConscript('#Core/SoarKernel/SConscript')
SConscript('#Core/ConnectionSML/SConscript')
SConscript('#Core/ElementXML/SConscript')
SConscript('#Core/CLI/SConscript')
SConscript('#Core/ClientSML/SConscript')
SConscript('#Core/KernelSML/SConscript')

if sys.platform == 'cygwin':
	SConscript('#Core/pcre/SConscript')

if env['java']:
	SConscript('#Core/ClientSMLSWIG/Java/SConscript')

	# Only build the interface if we're building for eclipse	
	if not env['eclipse']:
		SConscript('#Tools/LoggerJava/SConscript')
		SConscript('#Tools/TestJavaSML/SConscript')
		
		SConscript('#Tools/VisualSoar/SConscript')
			
		if env['swt']:
			SConscript('#Tools/SoarJavaDebugger/SConscript')
			SConscript('#Environments/Soar2D/SConscript')
			SConscript('#Environments/JavaMissionaries/SConscript')
			SConscript('#Environments/JavaTOH/SConscript')

if env['python']:
	SConscript('#Core/ClientSMLSWIG/Python/SConscript')

SConscript('#Tools/TestCLI/SConscript')
SConscript('#Tools/FilterC/SConscript')
SConscript('#Tools/QuickLink/SConscript')
SConscript('#Tools/SoarTextIO/SConscript')
SConscript('#Tools/TOHSML/SConscript')
SConscript('#Tools/TestSMLEvents/SConscript')
SConscript('#Tools/TestSMLPerformance/SConscript')
SConscript('#Tools/TestSoarPerformance/SConscript')

if not ( env['m64'] and ( env[ 'processor' ] == 'powerpc' ) ):
	SConscript('#Tests/SConscript')

