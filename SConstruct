#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>

SOAR_VERSION = "9.3.2"

DEF_OUT = 'out'
DEF_BUILD = 'build'

DEF_TARGETS = 'kernel cli sml_python sml_java debugger'.split()

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
	for f in execute(cc.split() + ['--version']).split():
		m = re.match(r'([0-9]+)\.([0-9]+)\.([0-9]+)', f)
		if m:
			return tuple(int(n) for n in m.groups())
		if f == 'clang':
			return [42,42,42]
	
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

AddOption('--cc', action='store', type='string', dest='cc', nargs=1, metavar='COMPILER',
	help='Use argument as the C compiler.')
	
AddOption('--cxx', action='store', type='string', dest='cxx', nargs=1, metavar='COMPILER',
	help='Use argument as the C++ compiler.')

AddOption('--cflags', action='store', type='string', dest='cflags', nargs=1, help='Compiler flags')

AddOption('--lnflags', action='store', type='string', dest='lnflags', nargs=1, help='Linker flags')

AddOption('--no-default-flags', action='store_false', dest='defflags', default=True, help="Don't pass any default flags to the compiler or linker")

AddOption('--no-scu', action='store_false', dest='scu', default=True,
	help='Don\'t build using single compilation units.')
	
AddOption('--out', action='store', type='string', dest='outdir', default=DEF_OUT, nargs=1, metavar='DIR',
	help='Directory to install binaries. Defaults to "out".')

AddOption('--build', action='store', type='string', dest='build-dir', default=DEF_BUILD, nargs=1, metavar='DIR',
	help='Directory to store intermediate (object) files. Defaults to "build".')

AddOption('--static', action='store_true', dest='static', default=False, help='Use static linking')

AddOption('--opt', action='store_true', dest='opt', default=False, help='Enable compiler optimizations, remove debugging symbols and assertions')

AddOption('--verbose', action='store_true', dest='verbose', default = False, help='Output full compiler commands')

env = Environment(
	ENV = os.environ.copy(),
	SCU = GetOption('scu'),
	BUILD_DIR = GetOption('build-dir'),
	OUT_DIR = os.path.realpath(GetOption('outdir')),
	SOAR_VERSION = SOAR_VERSION,
	VISHIDDEN = False,   # needed by swig
)

if GetOption('cc') != None:
	env.Replace(CC = GetOption('cc'))
elif sys.platform == 'darwin':
	env.Replace(CC = 'clang')
if GetOption('cxx') != None:
	env.Replace(CXX = GetOption('cxx'))
elif sys.platform == 'darwin':
	env.Replace(CXX = 'clang++')

print "Building intermediates to", env['BUILD_DIR']
print "Installing targets to", env['OUT_DIR']

config = Configure(env)

if 'g++' in env['CXX']:
	compiler = 'g++'
elif env['CXX'].endswith('cl') or (env['CXX'] == '$CC' and env['CC'].endswith('cl')):
	compiler = 'msvc'
else:
	compiler = os.path.split(env['CXX'])[1]

Export('compiler')

cflags = []
lnflags = []
libs = ['Soar']
if compiler == 'g++':
	if GetOption('defflags'):
		cflags.append('-Wreturn-type')
		
		if GetOption('opt'):
			cflags.extend(['-O2', '-DNDEBUG'])
		else:
			cflags.extend(['-g'])
		
		gcc_ver = gcc_version(env['CXX'])
		# check if the compiler supports -fvisibility=hidden (GCC >= 4)
		if gcc_ver[0] > 3:
			env.Append(CPPFLAGS='-fvisibility=hidden')
			if config.TryCompile('', '.cpp'):
				cflags.append('-fvisibility=hidden')
				cflags.append('-DGCC_HASCLASSVISIBILITY')
				env['VISHIDDEN'] = True
		
		if sys.platform == 'linux2':
			lnflags.append(env.Literal(r'-Wl,-rpath,$ORIGIN'))
		elif 'freebsd' in sys.platform:
			lnflags.append(env.Literal(r'-Wl,-z,origin,-rpath,$ORIGIN'))
		# For OSX, use -install_name (specified in Core/SConscript)
	
	libs += [ 'pthread' ]

elif compiler == 'msvc':
	cflags = ['/EHsc', '/D', '_CRT_SECURE_NO_DEPRECATE', '/D', '_WIN32', '/W2', '/bigobj']
	
	if GetOption('defflags'):
		if GetOption('opt'):
			cflags.extend(' /MD /O2 /D NDEBUG'.split())
		else:
			cflags.extend(' /MDd /Z7 /DEBUG'.split())
			lnflags.extend(['/DEBUG'])
	
	libs += ['advapi32']    # for GetUserName
			

cflags.extend((GetOption('cflags') or '').split())
lnflags.extend((GetOption('lnflags') or '').split())
	
env.Replace(
	CPPFLAGS = cflags,
	LINKFLAGS = lnflags,
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
	LIBS = libs,
	LIBPATH = [os.path.realpath(GetOption('outdir'))],
)

if sys.platform == 'win32':
	sys_lib_path = filter(None, os.environ.get('PATH', '').split(';'))
	sys_inc_path = filter(None, os.environ.get('INCLUDE', '').split(';'))
elif sys.platform == 'darwin':
	sys_lib_path = filter(None, os.environ.get('DYLD_LIBRARY_PATH', '').split(':'))
	sys_inc_path = filter(None, os.environ.get('CPATH', '').split(':'))
else:
	sys_lib_path = filter(None, os.environ.get('LD_LIBRARY_PATH', '').split(':'))
	sys_inc_path = filter(None, os.environ.get('CPATH', '').split(':'))

env.Append(CPPPATH = sys_inc_path, LIBPATH = sys_lib_path)

# Setting *COMSTR will replace long commands with a short message "Making <something>"
if not GetOption('verbose'):
	for x in 'CC SHCC CXX SHCXX LINK SHLINK JAR SWIG'.split():
		env[x + 'COMSTR'] = 'Making $TARGET'

	env['JAVACCOMSTR'] = 'Making $TARGET and others'

Export('env')

g_msvs_variant = 'Debug|Win32'

if 'MSVSSolution' in env['BUILDERS']:
	msvs_projs = []
	Export('msvs_projs')
	
	cl = subprocess.Popen('cl.exe /?', stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
	for line in cl.stdout:
		if re.search('x64', line):
			if GetOption('opt'):
				g_msvs_variant = 'Release|x64'
			else:
				g_msvs_variant = 'Debug|x64'
		else:
			if GetOption('opt'):
				g_msvs_variant = 'Release|Win32'
			else:
				g_msvs_variant = 'Debug|Win32'
 		break

Export('g_msvs_variant')

for d in os.listdir('.'):
	if not d.startswith('.'):
		script = join(d, 'SConscript')
		if os.path.exists(script):
			SConscript(script, variant_dir=join(GetOption('build-dir'), d), duplicate=0)

if 'MSVSSolution' in env['BUILDERS']:
	
	msvs_solution = env.MSVSSolution(
		target = 'soar' + env['MSVSSOLUTIONSUFFIX'],
		projects = msvs_projs,
		variant = g_msvs_variant,
	)
	
	env.Alias('msvs', [msvs_solution] + msvs_projs)

env.Alias('all', default_ans.keys())
all_aliases = default_ans.keys()

if COMMAND_LINE_TARGETS == ['list']:
	print '\n'.join(sorted(all_aliases))
	Exit()
	
# Set default targets
for a in DEF_TARGETS:
	if a in all_aliases:
		Default(a)
