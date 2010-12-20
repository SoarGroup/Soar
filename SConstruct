#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>
#
import os
import sys
import platform
import socket
import subprocess
import re

SOAR_VERSION = "9.3.1"

# host:                  winter,           seagull,          macsoar,       fugu,
# os.name:               posix,            posix,            posix,         posix,
# sys.platform:          linux2,           linux2,           darwin,        darwin,
# platform.machine:      x86_64,           i686,             i386,          Power Macintosh,
# platform.architecture: ('64bit', 'ELF'), ('32bit', 'ELF'), ('32bit', ''), ('32bit', '')
print socket.gethostname(), os.name, sys.platform, platform.machine(), platform.architecture()

if os.name not in ['posix', ]:
	print "Unsupported os.name:", os.name
	Exit(1)
if sys.platform not in ['linux2', 'darwin']:
	print "Unsupported sys.platform:", sys.platform
	Exit(1)
if platform.machine() not in ['x86_64', 'i686', 'i386', ]:
	print "Unsupported platform.machine:", platform.machine()

#################
# Option defaults 
def Mac_m64_Capable():
        proc = subprocess.Popen('sysctl -n hw.optional.x86_64', shell=True, stdout=subprocess.PIPE,)
        stdout_value = proc.communicate()[0]
        if proc.returncode != 0:
                return False
        if stdout_value.strip() == '1':
                return True
        return False

m64_default = '32'
if sys.platform == 'linux2':
	if platform.machine() == 'x86_64':
		m64_default = '64'
elif sys.platform == 'darwin':
	if platform.machine() == 'i386':
		if Mac_m64_Capable():
			m64_default = '64'

default_prefix = os.path.join('..','out')
if os.environ.has_key('SOAR_HOME'):
	default_prefix = os.environ['SOAR_HOME']

default_build = os.path.realpath(os.path.join('..','build'))
if os.environ.has_key('SOAR_BUILD'):
	default_build = os.environ['SOAR_BUILD']

################
# Command line options
AddOption('--cxx', action='store', type='string', dest='cxx', default='g++', nargs=1, metavar='COMPILER',
	help='Replace \'g++\' as the C++ compiler.')

AddOption('--build-warnings', action='store', type='choice', choices=['none','all','error'], dest='build-warnings', default='error', nargs=1, metavar='WARN_LEVEL',
	help='Set warning level when building. Must be one of none, all, error (default).')

AddOption('--optimization', action='store', type='choice', choices=['none','partial','full'], dest='optimization', default='full', nargs=1, metavar='LEVEL',
	help='Set optimization level. Must be one of none, partial, full (default).')

AddOption('--platform', action='store', type='choice', choices=['32','64'], dest='platform', default=m64_default, nargs=1, metavar='PLATFORM',
	help='Target platform. Must be one of 32, 64. Default is detected system architecture.')

AddOption('--build-verbose', action='store_true', dest='build-verbose', default=False,
	help='Build with verbose compiler output.')

AddOption('--preprocessor', action='store_true', dest='preprocessor', default=False,
	help='Only run preprocessor.')

AddOption('--gprof', action='store_true', dest='gprof', default=False,
	help='Add gprof symbols for profiling.')

AddOption('--no-scu', action='store_false', dest='scu', default=True,
	help='Don\'t build using single compilation units.')

# TODO: does this do the same thing as install-sandbox?
AddOption('--prefix', action='store', type='string', dest='prefix', default=default_prefix, nargs=1, metavar='DIR',
	help='Directory to install binaries. Defaults to "../out" (relative to SConstruct file).')

AddOption('--build-dir', action='store', type='string', dest='build-dir', default=default_build, nargs=1, metavar='DIR',
	help='Directory to store intermediate (object) files.')

AddOption('--no-debug-symbols', action='store_false', dest='debug-symbols', default=True,
	help='Don\'t add debugging symbols to binaries.')

AddOption('--static', action='store_true', dest='static', default=False,
	help='Use static linking (cannot use SWIG/Java/Python/CSharp/Tcl interfaces)')

#################
# Create the environment using the options
env = Environment(
	ENV = {'PATH' : os.environ['PATH']},
	CXX = GetOption('cxx'),
	SCU = GetOption('scu'), 
	BUILD_DIR = GetOption('build-dir'),
	PREFIX = os.path.realpath(GetOption('prefix')),
	STATIC_LINKED = GetOption('static'),
	SOAR_VERSION = SOAR_VERSION,
	CPPFLAGS = ['-DSCONS'],
	CPPPATH = ['#shared'],
	LINKFLAGS = [],
	)
#################


#################
# Get g++ Version
def gcc_version():
	proc = subprocess.Popen(env['CXX'] + ' --version ', shell=True, stdout=subprocess.PIPE)
	proc.wait()
	version_line = proc.stdout.readline().rsplit('\n', 1)[0]
	for possible_vs in version_line.split(' '):
		possible_svs = possible_vs.split('.')
		try:
			if len(possible_svs) is 3 and str(int(possible_svs[0])) and str(int(possible_svs[1])) and str(int(possible_svs[2])):
				split_version_string = possible_svs
				break
		except ValueError:
			continue
	return [int(split_version_string[0]), int(split_version_string[1]), int(split_version_string[2])]
gcc = gcc_version()
#################


#################
# Special configuration section
conf = Configure(env)
# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if gcc[0] > 3:
	print "Checking for visiblity=hidden...",
	lastCPPFLAGS = env['CPPFLAGS']
	env.Append(CPPFLAGS = ['-fvisibility=hidden'])
	if conf.TryCompile("char foo;", '.c'):
		print "yes"
		env['VISHIDDEN'] = True # needed by swig
	else:
		print "no"
		env['VISHIDDEN'] = False
		env.Replace(CPPFLAGS = lastCPPFLAGS)

# check for required libraries
if not conf.CheckLib('dl'):
	Exit(1)
	
if not conf.CheckLib('m'):
	Exit(1)

if not conf.CheckLib('pthread'):
	Exit(1)
		
# Check for optional libraries
env['HAS_TCL_LIB'] = conf.CheckLib('tcl8.4')
#################


#################
# CPPFLAGS, LINKFLAGS
# We need to know if we're on darwin because malloc.h doesn't exist, functions are in stdlib.h
if sys.platform == "darwin":
	env.Append(CPPFLAGS = ['-DSCONS_DARWIN'])

if env['STATIC_LINKED']:
	env.Append(CPPFLAGS = ['-DSTATIC_LINKED']);

# configure misc command line options
if GetOption('debug-symbols'):
	env.Append(CPPFLAGS = ['-g3'])

if GetOption('build-warnings') == 'all':
	env.Append(CPPFLAGS = ['-Wall'])
elif GetOption('build-warnings') == 'error':
	env.Append(CPPFLAGS = ['-Werror'])

if GetOption('optimization') == 'partial':
	env.Append(CPPFLAGS = ['-O2'])
elif GetOption('optimization') == 'full':
	env.Append(CPPFLAGS = ['-O3'])

if GetOption('preprocessor'):
	env.Append(CPPFLAGS = ['-E'])

if GetOption('build-verbose'):
	env.Append(CPPFLAGS = ['-v'])
	env.Append(LINKFLAGS = ['-v'])

if GetOption('platform') == '64':
	print "*"
	print "* Note: Targeting x86_64 (64-bit native)"
	print "*"
	env.Append(CPPFLAGS = Split('-m64 -fPIC'))
	env.Append(LINKFLAGS = ['-m64'])
elif gcc[0] > 4 or gcc[0] > 3 and gcc[1] > 1 and sys.platform not in [ 'darwin', ]:
	env.Append(CPPFLAGS = Split('-m32 -march=native'))
	env.Append(LINKFLAGS = Split('-m32 -march=native'))
else:
	env.Append(CPPFLAGS = ['-m32'])
	env.Append(LINKFLAGS = ['-m32'])

if GetOption('gprof'):
	env.Append(CPPFLAGS = ['-pg'])
	env.Append(LINKFLAGS = ['-pg'])
#################


#################
# Misc and export
env.Default(env['PREFIX'])
env.Default('.')

Export('env')
# TODO: rmdir PREFIX/share/soar on clean
#################


#################
# Fun Java Stuff
# TODO: Clean doesn't work quite right with jars
# theComponent: the top level folder name, just as JavaTOH
# theTargets: target jar name (or list of names) with -version.extension removed, makes .jar and .src.jar
# theSources: source folder (or list of folders) relative to component folder (such as 'src')
def javaRunAnt(theComponent, theTargets, theSources):
	if env['STATIC_LINKED']:
		print "Skipping Java component", theComponent
		return

	theDir = env.Dir('#../%s/' % theComponent)

	sharejava = env['PREFIX'] + '/share/java'
	javaSources = [sharejava + '/sml.jar']
	if type(theSources) == str:
		theSources = [theSources]
	for s in theSources:
		#print "walking:", os.path.join(str(theDir), s)
		for root, dirs, files in os.walk(os.path.join(str(theDir), s)):
			if ".svn" in dirs:
				dirs.remove(".svn")
			for f in files:
				#print "java dep:", os.path.join(root, f)
				javaSources.append(env.File(os.path.join(root, f)))

	javaSources.append(env.File(os.path.join(str(theDir), "build.xml")))

	ver = env['SOAR_VERSION']
	jarTargets = []
	if type(theTargets) == str:
		theTargets = [theTargets]
	for i in theTargets:
		targetRoot = sharejava + '/' + i + '-' + ver
		jarTargets.append(targetRoot + '.jar')
		jarTargets.append(targetRoot + '.src.jar')

	#print "-->", theComponent, jarTargets, javaSources
	ret = env.Command(jarTargets, javaSources, 'ant -q -Dsoarprefix=$PREFIX -Dversion=%s' % ver, chdir = theDir)

	if GetOption('clean'):
		env.Execute('ant -q -Dsoarprefix=$PREFIX clean -Dversion=' + env['SOAR_VERSION'], chdir = theDir)
	return ret
Export('javaRunAnt')
#################


#################
# InstallDir
# For recursion in to resource directories without grabbing .svn folders
# because Glob/InstallAs fails at life.
#
# Returns list of nodes for passage to Install
def InstallDir(comp, target, source, globstring="*"):
	targetdir = env.Dir(target)
	env.Clean(comp, targetdir)
	sourcedir = env.Dir(source)
	for root, dirs, files in os.walk(str(sourcedir)):
		if ".svn" in dirs:
			dirs.remove(".svn")
	
		# targetsub is the target directory plus the relative sub directory
		relative = root[len(str(sourcedir))+1:]
		targetsub = os.path.join(str(targetdir), relative)

		# sourceglob is all files in sub directory
		sourceglob = os.path.join(root, globstring)

		#print "install '%s' to '%s'" % (sourceglob, targetsub)
		env.Install(targetsub, env.Glob(sourceglob))
		for f in env.Glob(sourceglob, strings = True):
			(head, tail) = os.path.split(f)
			env.Clean(comp, os.path.join(targetsub, tail))
Export('InstallDir')
#################


#################
# Auto detect components
components = []
print "Detected components: ",
for root, dirs, files in os.walk('..'):
	if root != '..':
		del dirs[:]
	if root.startswith('../Core/') or root.startswith(env['PREFIX']) or root.startswith(env['BUILD_DIR']):
		continue;
	if 'SConscript' in files:
		component = root[3:]
		components.append(component)
		print component, 
print

if 'Tcl' in components:
	if not env['HAS_TCL_LIB']:
		print "* Removing Tcl from components (can't find tcl8.4 lib)"
		components.remove('Tcl')

# SWIG: build if any of (Java/Python/Tcl) are enabled
swig = False
for x in ['Java', 'Python', 'Tcl']:
	if x in components:
		if env['STATIC_LINKED']:
			components.remove(x)
			print "Removing component", x, " (static linking)"
		else:
			swig = True;
			print "Enabling SWIG."
			break

# As of 4/2009, python binaries available on mac are not x86_64 and
# therefore cannot load x86_64 targeted libraries. Verbosely warn.
if sys.platform == 'darwin':
	if GetOption('platform') == '64':
		if 'Python' in components:
			print "* Warning: 64-bit python binaries may not be available on your system."
			print "* You may need to rebuild with m64=no to use Python Soar bindings."
#################


#################
# Verify swig if enabled
def CheckSWIG(context):
	"""Checks to make sure we're using a compatible version of SWIG"""
	for line in os.popen("swig -version").readlines():
		m = re.search(r"SWIG Version ([0-9]+)\.([0-9]+)\.([0-9]+)", line)
		if m:
			ver = (int(m.group(1)), int(m.group(2)), int(m.group(3)))
			minver = (1, 3, 31)
			ret = 1
			status = 'ok'
			if ver < minver:
				ret = 0
				status = 'incompatible'
			print "Found SWIG version %d.%d.%d... %s" % (ver + (status,))
			return ret
	print "Didn't find SWIG, make sure SWIG is in the path and rebuild."
	return 0
if swig and not CheckSWIG(conf.env):
	Exit(1)
#################


#################
# Build core modules
subdirs= [ 
	'SoarKernel',
	'ConnectionSML',
	'ElementXML',
	'CLI',
	'ClientSML',
	'KernelSML',
	'Tests',
	]

if 'Python' in components:
	subdirs.append('ClientSMLSWIG/Python')

if 'Java' in components:
	subdirs.append('ClientSMLSWIG/Java')

if 'Tcl' in components:
	subdirs.append('ClientSMLSWIG/Tcl')

# Build/Output Directory
print "Building intermediates to directory ", env['BUILD_DIR']
print "Installing targets to prefix directory ", env['PREFIX']

for d in subdirs:
	script = '#%s/SConscript' % d
	print "Processing", script + "..."
	SConscript(script, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)

#################
# Components
# Special environment
compEnv = env.Clone()
compEnv.Prepend(CPPPATH = ['$PREFIX/include'])
libadd = ['ClientSML', 'ConnectionSML', 'ElementXML',]
if compEnv['STATIC_LINKED']:
        libadd = ['ClientSML', 'ConnectionSML', 'ElementXML', 'SoarKernelSML', 'SoarKernel', 'CommandLineInterface', 'sqlite3']
compEnv.Append(LIBS = libadd, LIBPATH = ['$PREFIX/lib'])
Export('compEnv')

# Add TestCLI to components (it uses compEnv which wasn't exported earlier):
components.insert(0, 'Core/TestCLI')

for d in components:
	script = '../%s/SConscript' % d
	if os.path.exists(script):
		print "Processing", script + "..."
		SConscript(script, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)
#################

