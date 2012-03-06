#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>

SOAR_VERSION = "9.3.1"

# required and default optional flags when using VC++ compiler
VS_REQ_CFLAGS = ' /EHsc /D _CRT_SECURE_NO_DEPRECATE /D _WIN32'
VS_DEF_CFLAGS = ' /O2 /W2'

# default compiler flags when using g++
GCC_DEF_CFLAGS = ' -O2 -Werror'

import os
import sys
import platform
import socket
import subprocess
import re
import fnmatch
from SCons.Node.Alias import default_ans

join = os.path.join

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

# Install all files under source directory to target directory, keeping
# subdirectory structure and ignoring hidden files
def InstallDir(env, tgt, src, globstring="*"):
	targets = []
	tgtdir = env.GetBuildPath(tgt)
	srcdir = env.GetBuildPath(src)
	for dir, _, files in os.walk(srcdir):
		if fnmatch.fnmatch(dir, '*/.*'):
			continue
		
		# tgtsub is the target directory plus the relative sub directory
		relative = dir[len(srcdir)+1:]
		tgtsub = join(tgtdir, relative)
		
		for f in fnmatch.filter(files, globstring):
			if not f.startswith('.'):
				p = join(dir, f)
				targets.extend(Install(tgtsub, p))
	
	return targets

Export('InstallDir')

# host:                  winter,           seagull,          macsoar,       fugu,
# os.name:               posix,            posix,            posix,         posix,
# sys.platform:          linux2,           linux2,           darwin,        darwin,
# platform.machine:      x86_64,           i686,             i386,          Power Macintosh,
# platform.architecture: ('64bit', 'ELF'), ('32bit', 'ELF'), ('32bit', ''), ('32bit', '')
print socket.gethostname(), os.name, sys.platform, platform.machine(), platform.architecture()

if os.name not in ['posix', 'nt']:
	print "Unsupported os.name:", os.name
	Exit(1)
if sys.platform not in ['linux2', 'darwin', 'win32']:
	print "Unsupported sys.platform:", sys.platform
	Exit(1)
if platform.machine() not in ['x86_64', 'AMD64', 'i686', 'i386', ]:
	print "Unsupported platform.machine:", platform.machine()


# Command line options
defarch = '32'
if platform.machine() in ['x86_64', 'AMD64'] or (sys.platform == 'darwin' and Mac_m64_Capable()):
	defarch = '64'

default_out = os.environ.get('SOAR_HOME', 'out')
default_build = os.environ.get('SOAR_BUILD', 'build')

AddOption('--cxx', action='store', type='string', dest='cxx', default='g++', nargs=1, metavar='COMPILER',
	help='Replace \'g++\' as the C++ compiler.')

AddOption('--cflags', action='store', type='string', dest='cflags', nargs=1, help='Compiler flags')

AddOption('--lnflags', action='store', type='string', dest='lnflags', nargs=1, help='Linker flags')

AddOption('--no-default-flags', action='store_false', dest='defflags', default=True, help="Don't pass any default flags to the compiler or linker")

AddOption('--arch', action='store', type='choice', choices=['32','64'], dest='platform', default=defarch, nargs=1, metavar='PLATFORM',
	help='Target architecture. Must be one of 32, 64. Default is detected system architecture.')

AddOption('--no-scu', action='store_false', dest='scu', default=True,
	help='Don\'t build using single compilation units.')
	
AddOption('--out-dir', action='store', type='string', dest='outdir', default=default_out, nargs=1, metavar='DIR',
	help='Directory to install binaries. Defaults to "../out" (relative to SConstruct file).')

AddOption('--build-dir', action='store', type='string', dest='build-dir', default=default_build, nargs=1, metavar='DIR',
	help='Directory to store intermediate (object) files.')

AddOption('--static', action='store_true', dest='static', default=False, help='Use static linking')

env = Environment(
	ENV = {
		'PATH' : os.environ.get('PATH', ''), 
		'TMP' : os.environ.get('TMP','')
	},
	SCU = GetOption('scu'), 
	BUILD_DIR = GetOption('build-dir'),
	OUT_DIR = os.path.realpath(GetOption('outdir')),
	SOAR_VERSION = SOAR_VERSION,
	VISHIDDEN = False,   # needed by swig
)

print "Building intermediates to", env['BUILD_DIR']
print "Installing targets to", env['OUT_DIR']

config = Configure(env)

if env['CXX'].endswith('g++'):
	compiler = 'g++'
elif env['CXX'].endswith('cl') or (env['CXX'] == '$CC' and env['CC'].endswith('cl')):
	compiler = 'cl'
else:
	compiler = os.path.split(env['CXX'])[1]

Export('compiler')

cflags = GetOption('cflags') or ''
if compiler == 'g++':
	# We need to know if we're on darwin because malloc.h doesn't exist, functions are in stdlib.h
	if sys.platform == 'darwin':
		cflags += ' -DSCONS_DARWIN'
	
	if GetOption('defflags'):
		if GetOption('cflags') == None:
			cflags += GCC_DEF_CFLAGS
		
		gcc_ver = gcc_version(env['CXX'])
		# check if the compiler supports -fvisibility=hidden (GCC >= 4)
		if gcc_ver[0] > 3:
			env.Append(CPPFLAGS='-fvisibility=hidden')
			if config.TryCompile('', '.cpp'):
				cflags += ' -fvisibility=hidden -DGCC_HASCLASSVISIBILITY'
				env['VISHIDDEN'] = True
		
		cflags += ' -mtune=native -m%s' % GetOption('platform')
	
elif compiler == 'cl':
	env.Append(LIBS='advapi32')  # for GetUserName
	cflags = VS_REQ_CFLAGS
	if GetOption('defflags') and GetOption('cflags') == None:
		cflags += VS_DEF_CFLAGS
	else:
		cflags += ' ' + GetOption('cflags')
	
env.Replace(
	CPPFLAGS=cflags.split(), 
	LINKFLAGS=(GetOption('lnflags') or "").split(),
	CPPPATH = [
		'#Core/shared',
		'#Core/pcre',
		'#Core/SoarKernel/src',
		'#Core/ElementXML/src',
		'#Core/KernelSML/src',
		'#Core/ConnectionSML/src',
		'#Core/ClientSML/src',
		'#Core/CLI/src',
	],
	LIBS = ['Soar'],
	LIBPATH = [os.path.realpath(GetOption('outdir'))],
)

if os.name == 'posix':
	if 'CPATH' in os.environ:
		env.Append(CPPPATH = os.environ.get('CPATH').split(':'))
	if sys.platform == 'darwin':
		lib_path_var = 'DYLD_LIBRARY_PATH'
	else:
		lib_path_var = 'LD_LIBRARY_PATH'
	if lib_path_var in os.environ:
		env.Append(LIBPATH = os.environ.get(lib_path_var).split(':'))

Export('env')

for d in os.listdir('.'):
	script = join(d, 'SConscript')
	if os.path.exists(script):
		SConscript(script, variant_dir=join(GetOption('build-dir'), d), duplicate=0)

all_aliases = default_ans.keys()

if COMMAND_LINE_TARGETS == ['list']:
	print '\n'.join(sorted(all_aliases))
	Exit()
	
# Set default targets
for a in 'kernel cli sml_python sml_java'.split():
	if a in all_aliases:
		Default(a)

if 'sml_java' in all_aliases:
	Default('debugger')
