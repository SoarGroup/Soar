#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>

import os
import sys
import platform
import socket
import subprocess
import re
import fnmatch

HEADER_DIRS = [
]

def execute(cmd):
	try:
		p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
	except OSError:
		print cmd[0], ' not in path'
		Exit(1)

	out = p.communicate()[0]
	if p.returncode != 0:
		print 'error executing ', cmd
		Exit(1)
	else:
		return out
	
def gcc_version(cc):
	for f in execute([cc, '--version']).split():
		m = re.match(r'([0-9]+)\.([0-9]+)\.([0-9]+)', f)
		if m:
			return tuple(int(n) for n in m.groups())
	
	print 'cannot identify compiler version'
	Exit(1)

def Mac_m64_Capable():
	return execute('sysctl -n hw.optional.x86_64'.split()).strip() == '1'

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
		for root, dirs, files in os.walk(os.path.join(str(theDir), s)):
			if ".svn" in dirs:
				dirs.remove(".svn")
			for f in files:
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

	ret = env.Command(jarTargets, javaSources, 'ant -q -Dsoarprefix=$PREFIX -Dversion=%s' % ver, chdir = theDir)

	if GetOption('clean'):
		env.Execute('ant -q -Dsoarprefix=$PREFIX clean -Dversion=' + env['SOAR_VERSION'], chdir = theDir)
	return ret
	
#################
# Verify swig if enabled
def CheckSWIG():
	for f in execute(['swig', '-version']).split():
		m = re.match(r'([0-9]+)\.([0-9]+)\.([0-9]+)', f)
		if m:
			ver = tuple(int(n) for n in m.groups())
			minver = (1, 3, 31)
			if ver < minver:
				print 'swig version 1.3.31 or higher is required'
				Exit(1)
			else:
				return
				
	print 'cannot determine swig version'
	Exit(1)

# Install all files under source directory to target directory, keeping
# subdirectory structure and ignoring hidden files
def InstallDir(env, tgt, src, globstring="*"):
	target = env.GetBuildPath(tgt)
	source = env.GetBuildPath(src)
	env.Clean('$PREFIX', target)
	for dir, _, files in os.walk(source):
		if fnmatch.fnmatch(dir, '*/.*'):
			continue
		
		# targetsub is the target directory plus the relative sub directory
		relative = dir[len(source)+1:]
		targetsub = os.path.join(target, relative)
		
		for f in fnmatch.filter(files, globstring):
			if not f.startswith('.'):
				p = os.path.join(dir, f)
				Install(targetsub, p)
	
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

m64_default = '32'
if platform.machine() == 'x86_64' or (sys.platform == 'darwin' and Mac_m64_Capable()):
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
	help='Use static linking (cannot use SWIG/Java/Python/CSharp/Tcl/PHP interfaces)')

AddOption('--ios', action='store', type='choice', choices=['none','simulator','armv6','armv7'], dest='ios', default='none', nargs=1, metavar='IOS',
		  help='Sets up for iOS compilation')

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
	CPPPATH = [
		'#shared',
		'#SoarKernel/src',
		'#ElementXML/src',
		'#KernelSML/src',
		'#ConnectionSML/src',
		'#ClientSML/src',
		'#Tests/src',
		'#CLI/src',
	],
	LINKFLAGS = [],
	)
#################

gcc = gcc_version(env['CXX'])
Export('gcc')
#################


#################
# Special configuration section
conf = Configure(env)
# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if gcc[0] > 3:
	lastCPPFLAGS = env['CPPFLAGS']
	env.Append(CPPFLAGS = ['-fvisibility=hidden'])
	if conf.TryCompile("char foo;", '.c'):
		env['VISHIDDEN'] = True # needed by swig
	else:
		env['VISHIDDEN'] = False
		env.Replace(CPPFLAGS = lastCPPFLAGS)

# check for required libraries
if not conf.CheckLib('dl'):
	print 'cannot locate libdl'
	Exit(1)
	
if not conf.CheckLib('m'):
	print 'cannot locate libm'
	Exit(1)

if not conf.CheckLib('pthread'):
	print 'cannot locate libpthread'
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

if GetOption('ios') == 'none':
	if GetOption('platform') == '64':
		env.Append(CPPFLAGS = Split('-m64 -fPIC'))
		env.Append(LINKFLAGS = ['-m64'])
	elif gcc[0] > 4 or gcc[0] > 3 and gcc[1] > 1 and sys.platform not in [ 'darwin', ]:
		env.Append(CPPFLAGS = Split('-m32 -march=native'))
		env.Append(LINKFLAGS = Split('-m32 -march=native'))
	else:
		env.Append(CPPFLAGS = ['-m32'])
		env.Append(LINKFLAGS = ['-m32'])
elif GetOption('ios') == 'simulator':
	env.Append(CPPFLAGS = Split('-m32 -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.0.sdk'))
	env.Append(LINKFLAGS = Split('-m32 -arch i386 -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator5.0.sdk'))
else:
	env.Append(CPPFLAGS = ['-DIPHONE_SDK', '-D__LLP64__']); # (the last is for STLSOFT)
	env['CC'] = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/llvm-gcc-4.2'
	env['CXX'] = '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/llvm-g++-4.2'
	env.Append(CPPFLAGS = Split('-arch ' + GetOption('ios') + ' -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.0.sdk'))
	env.Append(LINKFLAGS = Split('-arch ' + GetOption('ios') + ' -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.0.sdk'))

if GetOption('gprof'):
	env.Append(CPPFLAGS = ['-pg'])
	env.Append(LINKFLAGS = ['-pg'])
#################


#################
# Misc and export
env.Default(env['PREFIX'])
env.Default('.')

Export('env')
Export('javaRunAnt')


#################
# Auto detect components
components = []
print 'Detected components:',
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

# SWIG: build if any of (Java/Python/Tcl/PHP) are enabled
swig = False
for x in ['Java', 'Python', 'Tcl', 'PHP']:
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

if swig:
	CheckSWIG()

#################
# Build core modules
subdirs= [ 
	'SoarKernel',
	'ConnectionSML',
	'ElementXML',
	'CLI',
	'ClientSML',
	'KernelSML',
	]

if GetOption('ios') == 'none':
	subdirs.append('Tests')

	if 'Python' in components:
		subdirs.append('ClientSMLSWIG/Python')

	if 'Java' in components:
		subdirs.append('ClientSMLSWIG/Java')

	if 'Tcl' in components:
		subdirs.append('ClientSMLSWIG/Tcl')

	if 'PHP' in components:
		subdirs.append('ClientSMLSWIG/PHP')

# Build/Output Directory
print "Building intermediates to directory ", env['BUILD_DIR']
print "Installing targets to prefix directory ", env['PREFIX']

for d in subdirs:
	script = '#%s/SConscript' %d
	print "Processing", script + "..."
	SConscript(script, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)

#################
# Components
# Special environment
compEnv = env.Clone()
#compEnv.Prepend(CPPPATH = ['$PREFIX/include'])
libadd = ['ClientSML', 'ConnectionSML', 'ElementXML',]
if compEnv['STATIC_LINKED']:
	libadd = ['ClientSML', 'ConnectionSML', 'ElementXML', 'SoarKernelSML', 'SoarKernel', 'CommandLineInterface', 'sqlite3']
compEnv.Prepend(LIBS = libadd, LIBPATH = ['$PREFIX/lib'])
Export('compEnv')

# Add TestCLI to components (it uses compEnv which wasn't exported earlier):
components.insert(0, 'Core/TestCLI')
components.insert(0, 'Core/mincli')

if GetOption('ios') == 'none':
	for d in components:
		script = '../%s/SConscript' % d
		if os.path.exists(script):
			print "Processing", script + "..."
			SConscript(script, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)
#################

# Install resources
for d in ['#ElementXML/src', '#ConnectionSML/src', '#ClientSML/src', '#shared']:
	InstallDir(env, '$PREFIX/include', d, '*.h*')

env.Clean('$PREFIX', '$PREFIX/share')
InstallDir(env, '$PREFIX/share/soar/Demos', '#Demos')
InstallDir(env, '$PREFIX/share/soar/Documentation', '#Documentation')
InstallDir(env, '$PREFIX/share/soar/Tests', '#Tests/Agents')
