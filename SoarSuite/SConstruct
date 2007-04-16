#!/usr/bin/python
import os
import sys
import SoarSCons

# Custom test for GCC visibility flag
def CheckAndAddVisibilityFlag(context):
	context.Message('Checking support for -fvisibility=hidden... ')

	lastCPPFLAGS = context.env['CPPFLAGS']
	context.env.Append(CPPFLAGS = " -fvisibility=hidden")
	
	# 0 is failure (!)
	result = context.TryCompile("char foo;", '.c')
	if result == 0:
		context.env.Replace(CPPFLAGS = lastCPPFLAGS)

	context.Result(result)
	return result

env = Environment(CXXFLAGS = "-Wall -g", CPPPATH = ["#Core/shared",])
conf = Configure(env, custom_tests = {'CheckAndAddVisibilityFlag' : CheckAndAddVisibilityFlag})

def addSymbol(symbol):
	print "Adding symbol", symbol
	conf.env.Append(CPPFLAGS = ' -D%s' % symbol)

# What platform are we on?
print "Detected", os.name, "platform."
if os.name == 'posix':
	addSymbol("SCONS_POSIX")
	# we need the path to find java
	conf.env.Append(ENV = {'PATH' : os.environ['PATH']})
elif os.name == 'nt':
	# TODO: finish windows platform
	addSymbol("SCONS_NT")
	print "Platform unsupported."
	Exit(0)
else:
	# TODO: do osx platform if it isn't 'posix'
	print "Platform unsupported."
	Exit(0)
print

if not SoarSCons.ConfigureJNI(conf.env):
	print "Java Native Interface is required... Exiting"
	Exit(0)

# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if conf.env['CC'] == 'gcc':
	conf.CheckAndAddVisibilityFlag()
		
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

elif os.name == 'nt':
	# TODO: Many more things go here
	require('direct.h')
	require('windows.h')
print

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
SConscript('#Core/ClientSMLSWIG/Java/SConscript')
SConscript('#Core/ClientSMLSWIG/Python/SConscript')

# Test Apps
SConscript('#Tools/FilterC/SConscript')
# This needs some portability work before it can be added (malloc.h on osx)
##SConscript('#Tools/LoggerWinC/SConscript')
SConscript('#Tools/QuickLink/SConscript')
SConscript('#Tools/SoarJavaDebugger/SConscript')
SConscript('#Tools/SoarTextIO/SConscript')
SConscript('#Tools/TOHSML/SConscript')
SConscript('#Tools/TestCLI/SConscript')
SConscript('#Tools/TestClientSML/SConscript')
SConscript('#Tools/TestConnectionSML/SConscript')
SConscript('#Tools/TestJavaSML/SConscript')
SConscript('#Tools/TestMultiAgent/SConscript')
SConscript('#Tools/TestSMLEvents/SConscript')
SConscript('#Tools/TestSMLPerformance/SConscript')
SConscript('#Tools/TestSoarPerformance/SConscript')

