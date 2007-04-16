#!/usr/bin/python
import os
import sys
import SoarSCons

if os.name != "posix":
	print "Unsupported platform:", os.name
	Exit(1)

opts = Options()
opts.BoolOption('java', 'Set to 0 to not build the Soar Java interface', 1)
opts.BoolOption('python', 'Set to 0 to not build the Soar Python interface', 1)
opts.BoolOption('static', 'Set to 1 to statically link things when possible', 0)

env = Environment(options = opts,)
	
Help(opts.GenerateHelpText(env))
	
conf = Configure(
	env, 
	custom_tests = {'CheckVisibilityFlag' : SoarSCons.CheckVisibilityFlag})

conf.env.Append(CXXFLAGS = ['-Wall -g'])
conf.env.Append(CPPPATH = ['#Core/shared']
conf.env.Append(CPPFLAGS = ['-DSCONS'])
conf.env.Append(ENV = {'PATH' : os.environ['PATH']})

if opts['java']:
	if not SoarSCons.ConfigureJNI(conf.env):
		print "Java Native Interface is required... Exiting"
		print "It is possible to build without Java, see help."
		Exit(1)

# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if conf.CheckVisibilityFlag():
	conf.env.Append(CPPFLAGS = ['-fvisibility=hidden'])
		
# Require functions
def require(header):
	if not conf.CheckCXXHeader(header):
		print "Couldn't find required header", header
		Exit(0)
def requireLib(lib):
	if not conf.CheckLib(lib):
		print "Couldn't find required lib", lib
		Exit(0)

require('assert.h')
require('ctype.h')
require('math.h')
require('signal.h')
require('stdio.h')
require('time.h')

if os.name == 'posix':
	require('arpa/inet.h')
	require('ctype.h')
	require('dlfcn.h')
	require('errno.h')
	require('fcntl.h')
	require('inttypes.h')
	require('limits.h')
	require('locale.h')
	# Not on mac osx, associated functions are in stdlib.h
	#require('malloc.h')
	require('math.h')
	require('memory.h')
	require('netdb.h')
	require('netinet/in.h')
	require('pthread.h')
	require('signal.h')
	require('stddef.h')
	require('stdint.h')
	require('stdlib.h')
	require('string.h')
	require('strings.h')
	require('sys/param.h')
	require('sys/resource.h')
	require('sys/socket.h')
	require('sys/stat.h')
	require('sys/syscall.h')
	require('sys/time.h')
	require('sys/types.h')
	require('unistd.h')
	require('utime.h')

	requireLib('dl')
	requireLib('m')
	requireLib('pthread')

env = conf.Finish()
Export('env')

# Core
SConscript('#Core/SoarKernel/SConscript')
#SConscript('#Core/gSKI/SConscript')
#SConscript('#Core/ConnectionSML/SConscript')
#SConscript('#Core/ElementXML/SConscript')
#SConscript('#Core/CLI/SConscript')
#SConscript('#Core/ClientSML/SConscript')
#SConscript('#Core/KernelSML/SConscript')
#SConscript('#Core/ClientSMLSWIG/Java/SConscript')
#SConscript('#Core/ClientSMLSWIG/Python/SConscript')

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

