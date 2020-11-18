#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>

from __future__ import print_function

import os
import sys
import platform
import socket
import subprocess
import re
import fnmatch
from SCons.Node.Alias import default_ans
import SCons.Script
import shutil
import time

join = os.path.join

SOAR_VERSION = "9.6.0"

soarversionFile = open('soarversion', 'w')
print(SOAR_VERSION, file=soarversionFile)
soarversionFile.close()

DEF_OUT = 'out'
DEF_BUILD = 'build'
DEF_TARGETS = 'kernel cli sml_java debugger headers scripts'.split()

print("================================================================================")
print("Building Soar", SOAR_VERSION, "                      * will be built if no target specified")
print("Targets available:")
print("   Core:              kernel* cli* scripts*")
print("   Testing:           performance_tests tests")
print("   SWIG:              sml_python sml_tcl sml_java*")
print("   Extras:            debugger* headers* tclsoarlib")
print("Custom Settings available:                                              *default")
print("   Build Type:        --dbg, --opt*, --static")
print("   Custom Paths:      --out, --build, --tcl")
print("   Compilation time:  --no-svs, --scu*, --no-scu, --no-scu-kernel, --no-scu-cli")
print("   Customizations:    --cc, --cxx, --cflags, --lnflags, --no-default-flags, --verbose,")
print("================================================================================")

def execute(cmd):
    try:
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    except OSError:
        print(cmd[0], ' not in path')
        Exit(1)

    out = p.communicate()[0]
    if p.returncode != 0:
        print('error executing ', cmd)
        Exit(1)
    else:
        return out.decode()

def gcc_version(cc):
    version_info = execute(cc.split() + ['--version'])
    if 'GCC' in version_info or 'g++' in version_info:
        m = re.search(r'([0-9]+)\.([0-9]+)\.([0-9]+)', version_info)
        if m:
            return tuple(int(n) for n in m.groups())
    if 'clang' in version_info or 'LLVM' in version_info:
        return [42, 42, 42]

    print('cannot identify compiler version')
    Exit(1)

def vc_version():
    try:
        p = subprocess.Popen(['link.exe'], stdout=subprocess.PIPE, bufsize=1)
    except WindowsError as e:
        print("error running link.exe: {0}".format(e.strerror))
        print('make sure Microsoft Visual C++ is installed and you are using the Visual Studio Command Prompt')
        Exit(1)
    line = p.stdout.readline()
    # for line in iter(p.stdout.readline, b''):
    #     print(line,)
    p.communicate()
    m = re.search(r'Version ([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)', line.decode())
    if m:
        t = tuple(int(n) for n in m.groups())
        return str(t[0]) + '.' + str(t[1])

    print('cannot identify compiler version')
    Exit(1)

# run cl (MSVC compiler) and return the target architecture (x64 or x86)
def cl_target_arch():
    cl = subprocess.Popen('cl.exe /?', stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    for line in cl.stdout:
        if re.search('x64', line.decode()):
            return 'x64'
    return 'x86'

def Mac_m64_Capable():
    return execute('sysctl -n hw.optional.x86_64'.split()).strip() == '1'

# Install all files under source directory to target directory, keeping
# subdirectory structure and ignoring hidden files
def InstallDir(env, tgt, src, globstring="*"):
    targets = []
    tgtdir = env.GetBuildPath(tgt)
    srcdir = env.GetBuildPath(src)
    for dir, _, files in os.walk(srcdir):
        if fnmatch.fnmatch(dir[len(srcdir) + 1:], '*/.*'):
            continue

        # tgtsub is the target directory plus the relative sub directory
        relative = dir[len(srcdir) + 1:]
        tgtsub = join(tgtdir, relative)

        for f in fnmatch.filter(files, globstring):
            if not f.startswith('.'):
                p = join(dir, f)
                targets.extend(Install(tgtsub, p))

    return targets

def InstallDLLs(env):
  if sys.platform == 'win32' and not GetOption('dbg'):
    indlls = Glob(os.environ['VCINSTALLDIR'] + 'redist\\' + cl_target_arch() + '\\Microsoft.VC*.CRT\*')
    outdir = os.path.realpath(GetOption('outdir')) + '\\'
    if os.path.isfile(outdir):
        os.remove(outdir)
    if not os.path.exists(outdir):
        os.mkdir(outdir)
    for dll in indlls:
      # print('copy "' + dll.rstr() + '" "' + outdir + '"')
      shutil.copy(dll.rstr(), outdir)

Export('InstallDir')

AddOption('--cc', action='store', type='string', dest='cc', nargs=1, metavar='COMPILER', help='Use argument as the C compiler.')
AddOption('--cxx', action='store', type='string', dest='cxx', nargs=1, metavar='COMPILER', help='Use argument as the C++ compiler.')
AddOption('--cflags', action='store', type='string', dest='cflags', nargs=1, help='Compiler flags')
AddOption('--lnflags', action='store', type='string', dest='lnflags', nargs=1, help='Linker flags')
AddOption('--no-default-flags', action='store_false', dest='defflags', default=True, help="Don't pass any default flags to the compiler or linker")
AddOption('--no-scu', action='store_false', dest='scu', default=False, help='Don\'t build using single compilation units.')
AddOption('--no-scu-kernel', action='store_true', dest='no_scu_kernel', default=False, help='Never build kernel in a single compilation unit.')
AddOption('--no-scu-cli', action='store_true', dest='no_scu_cli', default=False, help='Never build CLI in a single compilation unit.')
AddOption('--scu', action='store_true', dest='scu', default=False, help='Build using single compilation units.')
AddOption('--out', action='store', type='string', dest='outdir', default=DEF_OUT, nargs=1, metavar='DIR', help='Directory to install binaries. Defaults to "out".')
AddOption('--build', action='store', type='string', dest='build-dir', default=DEF_BUILD, nargs=1, metavar='DIR', help='Directory to store intermediate (object) files. Defaults to "build".')
AddOption('--python', action='store', type='string', dest='python', default=sys.executable, nargs=1, help='Python executable')
AddOption('--tcl', action='store', type='string', dest='tcl', nargs=1, help='Active TCL (>= 8.6) libraries')
AddOption('--static', action='store_true', dest='static', default=False, help='Use static linking')
AddOption('--dbg', action='store_true', dest='dbg', default=False, help='Enable debug build.  Disables compiler optimizations, includes debugging symbols, debug trace statements and assertions')
AddOption('--opt', action='store_false', dest='dbg', default=False, help='Enable optimized build.  Enables compiler optimizations, removes debugging symbols, debug trace statements and assertions')
AddOption('--verbose', action='store_true', dest='verbose', default=False, help='Output full compiler commands')
AddOption('--no-svs', action='store_true', dest='nosvs', default=False, help='Build Soar without SVS functionality')

msvc_version = "12.0"
cl_target_architecture = ''
if sys.platform == 'win32':
    msvc_version = vc_version()
    cl_target_architecture = cl_target_arch()
    print("MSVC compiler target architecture is", cl_target_architecture)

env = Environment(
    MSVC_VERSION=msvc_version,
    ENV=os.environ.copy(),
    SCU=GetOption('scu'),
    NO_SCU_KERNEL=GetOption('no_scu_kernel'),
    NO_SCU_CLI=GetOption('no_scu_cli'),
    BUILD_DIR=GetOption('build-dir'),
    OUT_DIR=os.path.realpath(GetOption('outdir')),
    SOAR_VERSION=SOAR_VERSION,
    VISHIDDEN=False,  # needed by swig
	JAVAVERSION='1.8',
)

# This creates a file for cli_version.cpp to source.  For optimized builds, this guarantees
# that the build date will be correct in every build.  (Turned off for debug, b/c it was adding
# extra compilation time.  (for some reason, this will build it the first two times you compile after
# it exists.)

if ((GetOption('dbg') == None) or (GetOption('dbg') == False) or (FindFile('build_time_date.h', 'Core/shared/') == None)):
    cli_version_dep = open('Core/shared/build_time_date.h', 'w')
    print("const char* kTimestamp = __TIME__;", file=cli_version_dep)
    print("const char* kDatestamp = __DATE__;", file=cli_version_dep)
    print("//* Last build of Soar " + SOAR_VERSION + " occurred at " + time.ctime(time.time()) + " *//", file=cli_version_dep)
    cli_version_dep.close()
else:
    print("Build time stamp file was not built because this is a debug build.")
    
if GetOption('cc') != None:
    env.Replace(CC=GetOption('cc'))
elif sys.platform == 'darwin':
    env.Replace(CC='clang')
if GetOption('cxx') != None:
    env.Replace(CXX=GetOption('cxx'))
elif sys.platform == 'darwin':
    env.Replace(CXX='clang++')

print("Building intermediates to", env['BUILD_DIR'])
print("Installing targets to", env['OUT_DIR'])

if 'g++' in env['CXX']:
    compiler = 'g++'
elif env['CXX'].endswith('cl') or (env['CXX'] == '$CC' and env['CC'].endswith('cl')):
    compiler = 'msvc'
else:
    compiler = os.path.split(env['CXX'])[1]

lsb_build = ('lsbc++' in env['CXX'])
Export('compiler', 'lsb_build')

cflags = []
lnflags = []
libs = ['Soar']
if compiler == 'g++':
    libs += [ 'pthread', 'dl', 'm' ]
    if GetOption('nosvs'):
        cflags.append('-DNO_SVS')
    if GetOption('defflags'):
        cflags.append('-Wreturn-type')
        if GetOption('dbg'):
            cflags.extend(['-g'])
        else:
            cflags.extend(['-O3', '-DNDEBUG'])

        gcc_ver = gcc_version(env['CXX'])
        # check if the compiler supports -fvisibility=hidden (GCC >= 4)
        if gcc_ver[0] > 3:
            env.Append(CPPFLAGS='-fvisibility=hidden')
            
            config = Configure(env)
            if config.TryCompile('', '.cpp'):
                cflags.append('-fvisibility=hidden')
                cflags.append('-DGCC_HASCLASSVISIBILITY')
                env['VISHIDDEN'] = True
            else:
                env['VISHIDDEN'] = False
                env['CPPFLAGS'] = []
            config.Finish()

        if sys.platform.startswith('linux'):
            lnflags.append(env.Literal(r'-Wl,-rpath,$ORIGIN'))
            libs.append('rt')
        elif 'freebsd' in sys.platform:
            lnflags.append(env.Literal(r'-Wl,-z,origin,-rpath,$ORIGIN'))
        # For OSX, use -install_name (specified in Core/SConscript)

        if GetOption('static'):
            cflags.extend(['-DSTATIC_LINKED', '-fPIC'])

elif compiler == 'msvc':
    cflags = ['/EHsc', '/D', '_CRT_SECURE_NO_DEPRECATE', '/D', '_WIN32', '/W2', '/bigobj']
    if GetOption('nosvs'):
        cflags.extend(' /D NO_SVS'.split())
    if GetOption('defflags'):
        if GetOption('dbg'):
            cflags.extend(' /MDd /Z7 /DEBUG'.split())
            lnflags.extend(['/DEBUG'])
        else:
            cflags.extend(' /MD /O2 /D NDEBUG'.split())

        if GetOption('static'):
            cflags.extend(['/D', 'STATIC_LINKED'])

cflags.extend((GetOption('cflags') or '').split())
lnflags.extend((GetOption('lnflags') or '').split())

env.Replace(
    CPPFLAGS=cflags,
    LINKFLAGS=lnflags,
    CPPPATH=[
        '#Core/shared',
        '#Core/SoarKernel/src/debug_code',
        '#Core/SoarKernel/src/decision_process',
        '#Core/SoarKernel/src/episodic_memory',
        '#Core/SoarKernel/src/explanation_memory',
        '#Core/SoarKernel/src/explanation_based_chunking',
        '#Core/SoarKernel/src/interface',
        '#Core/SoarKernel/src/output_manager',
        '#Core/SoarKernel/src/parsing',
        '#Core/SoarKernel/src/reinforcement_learning',
        '#Core/SoarKernel/src/semantic_memory',
        '#Core/SoarKernel/src/shared',
        '#Core/SoarKernel/src/soar_representation',
        '#Core/SoarKernel/src/visualizer',
        '#Core/ElementXML/src',
        '#Core/KernelSML/src',
        '#Core/ConnectionSML/src',
        '#Core/ClientSML/src',
        '#Core/CLI/src',
    ],
    LIBS=libs,
    LIBPATH=[os.path.realpath(GetOption('outdir'))],
)

if sys.platform == 'win32':
    sys_lib_path = list(filter(None, os.environ.get('PATH', '').split(';')))
    sys_inc_path = list(filter(None, os.environ.get('INCLUDE', '').split(';')))
elif sys.platform == 'darwin':
    sys_lib_path = list(filter(None, os.environ.get('DYLD_LIBRARY_PATH', '').split(':')))
    sys_inc_path = list(filter(None, os.environ.get('CPATH', '').split(':')))
else:
    sys_lib_path = list(filter(None, os.environ.get('LD_LIBRARY_PATH', '').split(':')))
    sys_inc_path = list(filter(None, os.environ.get('CPATH', '').split(':')))

if sys.platform != 'win32':
    env.Append(CXXFLAGS='-std=c++11')

if not sys.platform == 'darwin':
    env.Append(CPPPATH=sys_inc_path, LIBPATH=sys_lib_path)

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

    if (cl_target_architecture == 'x64'):
        if GetOption('dbg'):
            g_msvs_variant = 'Debug|x64'
        else:
            g_msvs_variant = 'Release|x64'
    else:
        if GetOption('dbg'):
            g_msvs_variant = 'Debug|Win32'
        else:
            g_msvs_variant = 'Release|Win32'

Export('g_msvs_variant')

for d in os.listdir('.'):
    if not d.startswith('.'):
        script = join(d, 'SConscript')
        if os.path.exists(script):
            SConscript(script, variant_dir=join(GetOption('build-dir'), d), duplicate=0)

if 'MSVSSolution' in env['BUILDERS']:

    msvs_solution = env.MSVSSolution(
        target='soar' + env['MSVSSOLUTIONSUFFIX'],
        projects=msvs_projs,
        variant=g_msvs_variant,
    )

    env.Alias('msvs', [msvs_solution] + msvs_projs)

env.Alias('all', list(default_ans.keys()))
all_aliases = default_ans.keys()

if COMMAND_LINE_TARGETS == ['list']:
    print('\n'.join(sorted(all_aliases)))
    Exit()

# Set default targets
for a in DEF_TARGETS:
    if a in all_aliases:
        Default(a)

InstallDLLs(env)
