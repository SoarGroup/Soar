#!/usr/bin/python
import os
import sys
import SoarSCons

if os.name != "posix":
	print "Unsupported platform:", os.name
	Exit(1)

opts = Options()
opts.AddOptions(
	BoolOption('java', 'Build the Soar Java interface', 'yes'), 
	BoolOption('python', 'Build the Soar Python interface', 'yes'), 
	BoolOption('static', 'Use static linking when possible', 'no'), 
	BoolOption('debug', 'Build with debugging symbols', 'yes'),
	BoolOption('warnings', 'Build with warnings', 'yes'),
	BoolOption('csharp', 'Build the Soar CSharp interface', 'no'), 
	BoolOption('tcl', 'Build the Soar Tcl interface', 'no'), 
)

env = Environment(options = opts)
Help(opts.GenerateHelpText(env))

env.Append(CPPPATH = ['#Core/shared'])
env.Append(CPPFLAGS = ' -DSCONS')
env.Append(ENV = {'PATH' : os.environ['PATH']})

# Create configure context to configure the environment
custom_tests = {
	'CheckVisibilityFlag' : SoarSCons.CheckVisibilityFlag,
}
conf = Configure(env, custom_tests = custom_tests)

# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if conf.CheckVisibilityFlag():
	conf.env.Append(CPPFLAGS = ' -fvisibility=hidden')

# configure misc command line options
if conf.env['debug']:
	conf.env.Append(CPPFLAGS = ' -g')
if conf.env['warnings']:
	conf.env.Append(CPPFLAGS = ' -Wall')

# configure java
if conf.env['java']:
	if not SoarSCons.ConfigureJNI(conf.env):
		print "Could not configure Java. If you know where java is on your system,"
		print "set environment variable JAVA_HOME to point to the directory containing"
		print "the Java include, bin, and lib directories."
		print "Java Native Interface is required... Exiting"
		Exit(1)

# check SWIG version if necessary
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

# check for required libraries
if not conf.CheckLib('dl'):
	Exit(1)
	
if not conf.CheckLib('m'):
	Exit(1)

if not conf.CheckLib('pthread'):
	Exit(1)

env = conf.Finish()
Export('env')

# Core
SConscript('#Core/SoarKernel/SConscript')
SConscript('#Core/gSKI/SConscript')
SConscript('#Core/ConnectionSML/SConscript')
SConscript('#Core/ElementXML/SConscript')
SConscript('#Core/CLI/SConscript')
SConscript('#Core/ClientSML/SConscript')
SConscript('#Core/KernelSML/SConscript')

if env['java']:
	SConscript('#Core/ClientSMLSWIG/Java/SConscript')

if env['python']:
	SConscript('#Core/ClientSMLSWIG/Python/SConscript')

# Test Apps
#SConscript('#Tools/FilterC/SConscript')
# This needs some portability work before it can be added (malloc.h on osx)
##SConscript('#Tools/LoggerWinC/SConscript')
#SConscript('#Tools/QuickLink/SConscript')
#SConscript('#Tools/SoarJavaDebugger/SConscript')
#SConscript('#Tools/SoarTextIO/SConscript')
#SConscript('#Tools/TOHSML/SConscript')
#SConscript('#Tools/TestCLI/SConscript')
#SConscript('#Tools/TestClientSML/SConscript')
#SConscript('#Tools/TestConnectionSML/SConscript')
#SConscript('#Tools/TestJavaSML/SConscript')
#SConscript('#Tools/TestMultiAgent/SConscript')
#SConscript('#Tools/TestSMLEvents/SConscript')
#SConscript('#Tools/TestSMLPerformance/SConscript')
#SConscript('#Tools/TestSoarPerformance/SConscript')

