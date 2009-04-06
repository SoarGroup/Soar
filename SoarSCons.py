#
# This file is a bunch of utility functions for the SConstruct file.


# swt.jar 3.4 digests
OSX_DIGEST = '7dab557faeb4e182281be824baebd856'
GTK_DIGEST = '28b79f2cb0bc09c1efd4b711c0cd52a7'
GTK64_DIGEST = '69358481ec9d44b068bfe1dc1da57737'

import os
import sys
import string
import re
import shutil
import urllib
import md5

def DoCopy(target, source, env):
	target_files = map(lambda x: str(x), target)
	source_files = map(lambda x: str(x), source)
	
        for target, source in zip(target_files, source_files):
		shutil.copyfile(source, target)

def SetJavaPaths(env, classpath, sourcepath = None):
	if sourcepath == None:
		# use the default
		sourcepath = '${SOURCE.dir.rdir()}'

	# Set up java command to use classpath and sourcepath
	env['JAVACCOM'] = '$JAVAC $JAVACFLAGS -d ${TARGET.attributes.java_classdir} -classpath $CLASSPATH -sourcepath ' + sourcepath + ' $SOURCES'

	if os.name == 'nt':
		classpath = classpath.replace('/', '\\')
		classpath = classpath.replace(':', ';')
	env['CLASSPATH'] = classpath

def CheckJarmd5(env, jarpath):
	# open the swt.jar file
	try:
		f = file(jarpath)
	except:
		return False
	
	# compute digest
	m = md5.new()
	while True:
	    d = f.read(8096)
	    if not d:
	        break
	    m.update(d)
	    
	if sys.platform == 'darwin':
		return OSX_DIGEST == m.hexdigest()
	else:
		if env['m64']:
			return GTK64_DIGEST == m.hexdigest()
		else:
			return GTK_DIGEST == m.hexdigest()
	
def CheckForSWTJar(env):
	jarpath = os.path.join('SoarLibrary', 'lib', 'swt.jar')
	if os.path.exists(jarpath):
		if CheckJarmd5(env, jarpath):
			return True
		else:
			print "md5 of swt.jar failed, removing old jar."
			os.remove(jarpath)
		
	try:
		if sys.platform == 'darwin':
			urllib.urlretrieve('http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/osx/swt.jar', jarpath)
		else:
			if env['m64']:
				urllib.urlretrieve('http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/gtk64/swt.jar', jarpath)
			else:
				urllib.urlretrieve('http://ai.eecs.umich.edu/~soar/sitemaker/misc/jars/gtk32/swt.jar', jarpath)
	except IOError:
		print "Error downloading %s: IOError" % jarpath
		return False
	except ContentTooShortError:
		print "Error downloading %s: IOError" % jarpath
		return False
		
	if not CheckJarmd5(env, jarpath):
		print "Error downloading %s, md5 failed again." % jarpath
		return False
	
	print "Successfully downloaded", jarpath
	return True

def osx_copy(dest, source, env):
	from macostools import copy
	copy(source, dest)

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

    #if not env.get('JAVAC'):
    #    print "The Java compiler must be installed and in the current path."
    #    return 0

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

    if sys.platform == 'darwin':
        # Apple does not use Sun's naming convention
        java_headers = [os.path.join(java_base, 'include')]
        java_libs = [os.path.join(java_base, 'lib')]
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
    return 1

def JavaSources(dir):
	sources = list()
	for root, dirs, files in os.walk(dir):
		for f in files:
			sources.append(os.path.join(root, f))
		if '.svn' in dirs:
			dirs.remove('.svn')
	return sources

