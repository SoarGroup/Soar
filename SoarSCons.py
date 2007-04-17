#
# This file is a bunch of utility functions for the SConstruct file.

import os
import sys
import string
import re

def CheckVisibilityFlag(context):
	"""Checks to see if the compiler will build using the visibility flag"""
	context.Message('Checking support for -fvisibility=hidden... ')

	# save old flags
	lastCPPFLAGS = context.env['CPPFLAGS']
	
	# add new flag
	context.env.Append(CPPFLAGS = ' -fvisibility=hidden')

	# try compiling simple file with new flag
	result = context.TryCompile("char foo;", '.c')

	# restore old flags
	context.env.Replace(CPPFLAGS = lastCPPFLAGS)

	# print status message and return
	context.Result(result)
	return result

def CheckSWIG(context):
	"""Checks to make sure we're using a compatible version of SWIG"""
	for line in os.popen("swig -version").readlines():
		m = re.search(r"([0-9])\.([0-9])\.([0-9]?[0-9])", line)
		if m:
			major = int(m.group(1))
			minor = int(m.group(2))
			micro = int(m.group(3))

			ret = 1
			status = 'ok'
			if major < 1 or minor < 3 or micro < 31:
				ret = 0
				status = 'incompatible'
			print "Found SWIG version %d.%d.%d... %s" % (major, minor, micro, status)
			return ret
	print "Didn't find SWIG, make sure SWIG is in the path and rebuild."
	return 0
	
# ConfigureJNI and tolen from http://www.scons.org/wiki/JavaNativeInterface
# Modified from its original format

def walkDirs(path):
    """helper function to get a list of all subdirectories"""
    def addDirs(pathlist, dirname, names):
        """internal function to pass to os.path.walk"""
        for n in names:
            f = os.path.join(dirname, n)
            if os.path.isdir(f):
                pathlist.append(f)
    pathlist = [path]
    os.path.walk(path, addDirs, pathlist)
    return pathlist

def ConfigureJNI(env):
    """Configure the given environment for compiling Java Native Interface
       c or c++ language files."""

    if not env.get('JAVAC'):
        print "The Java compiler must be installed and in the current path."
        return 0

    # first look for a shell variable called JAVA_HOME
    java_base = os.environ.get('JAVA_HOME')
    if not java_base:
        if sys.platform == 'darwin':
            # Apple's OS X has its own special java base directory
            java_base = '/System/Library/Frameworks/JavaVM.framework'
        else:
            # Search for the java compiler
            print "JAVA_HOME environment variable is not set. Searching for java... ",
            jcdir = os.path.dirname(env.WhereIs('javac'))
            if not jcdir:
                print "not found."
                return 0
            # assuming the compiler found is in some directory like
            # /usr/jdkX.X/bin/javac, java's home directory is /usr/jdkX.X
            java_base = os.path.join(jcdir, "..")
            print "found:", java_base

    if sys.platform == 'cygwin':
        # Cygwin and Sun Java have different ideas of how path names
        # are defined. Use cygpath to convert the windows path to
        # a cygwin path. i.e. C:\jdkX.X to /cygdrive/c/jdkX.X
        java_base = string.replace( \
                os.popen("cygpath -up '"+java_base+"'").read(), '\n', '')

    if sys.platform == 'darwin':
        # Apple does not use Sun's naming convention
        java_headers = [os.path.join(java_base, 'Headers')]
        java_libs = [os.path.join(java_base, 'Libraries')]
    else:
        # windows and linux
        java_headers = [os.path.join(java_base, 'include')]
        java_libs = [os.path.join(java_base, 'lib')]
        # Sun's windows and linux JDKs keep system-specific header
        # files in a sub-directory of include
        if java_base == '/usr' or java_base == '/usr/local':
            # too many possible subdirectories. Just use defaults
            java_headers.append(os.path.join(java_headers[0], 'win32'))
            java_headers.append(os.path.join(java_headers[0], 'linux'))
            java_headers.append(os.path.join(java_headers[0], 'solaris'))
        else:
            # add all subdirs of 'include'. The system specific headers
            # should be in there somewhere
            java_headers = walkDirs(java_headers[0])

    # add Java's include and lib directory to the environment
    env.Append(CPPPATH = java_headers)
    env.Append(LIBPATH = java_libs)

    # add any special platform-specific compilation or linking flags
    if sys.platform == 'darwin':
        env.Append(SHLINKFLAGS = '-dynamiclib -framework JavaVM')
        env['SHLIBSUFFIX'] = '.jnilib'
    elif sys.platform == 'cygwin':
        env.Append(CCFLAGS = '-mno-cygwin')
        env.Append(SHLINKFLAGS = '-mno-cygwin -Wl,--kill-at')

    # Add extra potentially useful environment variables
    env['JAVA_HOME'] = java_base
    env['JNI_CPPPATH'] = java_headers
    env['JNI_LIBPATH'] = java_libs
    return 1

