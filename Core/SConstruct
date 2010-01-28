#!/usr/bin/python
import os
import sys
import platform
import socket
import subprocess
import re

SOAR_VERSION = "9.2.1"

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

default_prefix = os.path.realpath(os.path.join('..','out'))
if os.environ.has_key('SOAR_HOME'):
	default_prefix = os.environ['SOAR_HOME']

default_build = os.path.realpath(os.path.join('..','build'))
if os.environ.has_key('SOAR_BUILD'):
	default_build = os.environ['SOAR_BUILD']

################
# Command line options
AddOption('--cxx',
	action='store',
	type='string',
	dest='cxx',
	default='g++',
	nargs=1,
	metavar='COMPILER',
	help='Replace \'g++\' as the C++ compiler.')

AddOption('--build-warnings',
	action='store',
	type='choice',
	choices=['none','all','error'],
	dest='build-warnings',
	default='error',
	nargs=1,
	metavar='WARN_LEVEL',
	help='Set warning level when building. Must be one of none, all, error (default).')

AddOption('--optimization',
	action='store',
	type='choice',
	choices=['none','partial','full'],
	dest='optimization',
	default='full',
	nargs=1,
	metavar='LEVEL',
	help='Set optimization level. Must be one of none, partial, full (default).')

AddOption('--platform',
	action='store',
	type='choice',
	choices=['32','64'],
	dest='platform',
	default=m64_default,
	nargs=1,
	metavar='PLATFORM',
	help='Target platform. Must be one of 32, 64. Default is detected system architecture.')

AddOption('--build-verbose', 
	action='store_true',
	dest='build-verbose',
	default=False,
	help='Build with verbose compiler output.')

AddOption('--preprocessor', 
	action='store_true',
	dest='preprocessor',
	default=False,
	help='Only run preprocessor.')

AddOption('--gprof', 
	action='store_true',
	dest='gprof',
	default=False,
	help='Add gprof symbols for profiling.')

AddOption('--no-scu', 
	action='store_false',
	dest='scu',
	default=True,
	help='Don\'t build using single compilation units.')

# TODO: does this do the same thing as install-sandbox?
AddOption('--prefix',
	action='store',
	type='string',
	dest='prefix',
	default=default_prefix,
	nargs=1,
	metavar='DIR',
	help='Directory to install binaries.')

AddOption('--build-dir',
	action='store',
	type='string',
	dest='build-dir',
	default=default_build,
	nargs=1,
	metavar='DIR',
	help='Directory to store intermediate (object) files.')

AddOption('--no-debug-symbols',
	action='store_false',
	dest='debug-symbols',
	default=True,
	help='Don\'t add debugging symbols to binaries.')

# Create the environment using the options
env = Environment(
	ENV = os.environ, 
	CXX = GetOption('cxx'),
	BUILD_WARNINGS = GetOption('build-warnings'),
	OPTIMIZATION = GetOption('optimization'),
	PLATFORM = GetOption('platform'),
	BUILD_VERBOSE = GetOption('build-verbose'),
	PREPROCESSOR = GetOption('preprocessor'),
	GPROF = GetOption('gprof'),
	SCU = GetOption('scu'), 
	BUILD_DIR = GetOption('build-dir'),
	PREFIX = GetOption('prefix'),
	DEBUG_SYMBOLS = GetOption('debug-symbols'),
	)

##################
# Create configure context to configure the environment
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

custom_tests = {
	# A check to see if the -fvisibility flag works on this system.
	'CheckVisibilityFlag' : CheckVisibilityFlag,
}
conf = Configure(env, custom_tests = custom_tests)

conf.env.Append(SOAR_VERSION = SOAR_VERSION)

# We define SCONS to indicate to the source that SCONS is controlling the build.
conf.env.Append(CPPFLAGS = ' -DSCONS')

# We need to know if we're on darwin because malloc.h doesn't exist, functions are in stdlib.h
if sys.platform == "darwin":
	conf.env.Append(CPPFLAGS = ' -DSCONS_DARWIN')
	#if conf.env['ppc']:
		#conf.env.Append(CPPFLAGS = ' -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc ')
		#conf.env.Append(LINKFLAGS = ' -isysroot /Developer/SDKs/MacOSX10.5.sdk -arch ppc ')

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

# All C/C++ modules rely or should rely on this include path (houses portability.h)
conf.env.Append(CPPPATH = ['#shared'])

# check if the compiler supports -fvisibility=hidden (GCC >= 4)
if conf.CheckVisibilityFlag() and gcc[0] > 3:
	conf.env.Append(CPPFLAGS = ' -fvisibility=hidden')

# configure misc command line options
if conf.env['DEBUG_SYMBOLS']: 
	conf.env.Append(CPPFLAGS = ' -g3')

if conf.env['BUILD_WARNINGS'] == 'all':
	conf.env.Append(CPPFLAGS = ' -Wall')
elif conf.env['BUILD_WARNINGS'] == 'error':
	conf.env.Append(CPPFLAGS = ' -Werror')

if conf.env['OPTIMIZATION'] == 'partial':
	conf.env.Append(CPPFLAGS = ' -O2')
elif conf.env['OPTIMIZATION'] == 'full':
	conf.env.Append(CPPFLAGS = ' -O3')

if conf.env['PREPROCESSOR']:
	conf.env.Append(CPPFLAGS = ' -E')

if conf.env['BUILD_VERBOSE']:
	conf.env.Append(CPPFLAGS = ' -v')
	conf.env.Append(LINKFLAGS = ' -v')

# check for required libraries
if not conf.CheckLib('dl'):
	Exit(1)
	
if not conf.CheckLib('m'):
	Exit(1)

if not conf.CheckLib('pthread'):
	Exit(1)
		
conf.env['HAS_TCL_LIB'] = conf.CheckLib('tcl8.4')
		
if conf.env['PLATFORM'] == '64':
	print "*"
	print "* Note: Targeting x86_64 (64-bit native)"
	print "*"
	conf.env.Append(CPPFLAGS = ' -m64 -fPIC')
	conf.env.Append(LINKFLAGS = ' -m64')
elif gcc[0] > 4 or gcc[0] > 3 and gcc[1] > 1:
	conf.env.Append(CPPFLAGS = ' -m32 -march=native')
	conf.env.Append(LINKFLAGS = ' -m32 -march=native')
else:
	conf.env.Append(CPPFLAGS = ' -m32')
	conf.env.Append(LINKFLAGS = ' -m32')

if conf.env['GPROF']:
	conf.env.Append(CPPFLAGS = ' -pg')
	conf.env.Append(LINKFLAGS = ' -pg')

#################
# Default targets
conf.env.Default(conf.env['PREFIX'])
conf.env.Default('.')
# TODO: rmdir PREFIX/share/soar on clean

env = conf.Finish()
Export('env')

#################
# Auto detect components
components = []
print "Detected components: ",
for root, dirs, files in os.walk('..'):
	if root != '..':
		del dirs[:]
	if root.startswith('../Core') or root.startswith(env['PREFIX']) or root.startswith(env['BUILD_DIR']):
		continue;
	if 'SConscript' in files:
		component = root[3:]
		components.append(component)
		print component + ' ',
print

if 'Tcl' in components:
	if not env['HAS_TCL_LIB']:
		print "*"
		print "* Removing Tcl from components (can't find tcl8.4 lib)"
		print "*"
		components.remove('Tcl')

# SWIG: build if any of (Java/Python/Tcl) are enabled
swig = False
for x in ['Java', 'Python', 'Tcl']:
	if x in components:
		swig = True;
		print "Enabling SWIG."
		break

# As of 4/2009, python binaries available on mac are not x86_64 and
# therefore cannot load x86_64 targeted libraries. Verbosely warn.
if sys.platform == 'darwin':
	if env['PLATFORM'] == '64':
		if 'Python' in components:
			print "*"
			print "* Warning: 64-bit python binaries may not be available on your system."
			print "* You may need to rebuild with m64=no to use Python Soar bindings."
			print "*"
#################

#################
# New configuration context for swig projects
if swig:
	conf = Configure(env)
	
	# This allows us to use Java and SWIG if they are in the path.
	conf.env.Append(ENV = {'PATH' : os.environ['PATH']})
	
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
	
	if not CheckSWIG(conf.env):
		Exit(1)
	
	swigenv = conf.Finish()
	Export('swigenv')
#################

#################
# Build modules
subdirs= [ 
	'SoarKernel',
	'ConnectionSML',
	'ElementXML',
	'CLI',
	'ClientSML',
	'KernelSML',
	'Tests',
	'TestCLI',
	'TestSMLEvents',
	'TestSMLPerformance',
	'TestSoarPerformance',
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
	SConscript('#%s/SConscript' % d, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)

for d in components:
	script = '../%s/SConscript' % d
	if os.path.exists(script):
		print "Processing", script + "..."
		SConscript(script, variant_dir=os.path.join(env['BUILD_DIR'], d), duplicate=0)

