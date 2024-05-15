#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>

from __future__ import print_function

import os
import sys
import subprocess
import re
import fnmatch
from SCons.Node.Alias import default_ans
import time

try:
    enscons_active = True
    import toml
    import enscons, enscons.cpyext
except ImportError as e:
    enscons_active = False

# Add the current directory to the path so we can from build_support
script_dir = Dir('.').srcnode().abspath
sys.path.append(script_dir)
from build_support.tcl import prepare_for_compiling_with_tcl

join = os.path.join

SOAR_VERSION = "9.6.2"
CPP_STD_VERSION = "c++17"

soarversionFile = open('soarversion', 'w')
print(SOAR_VERSION, file=soarversionFile)
soarversionFile.close()

MSVS_ALIAS = 'msvs'
COMPILE_DB_ALIAS = 'cdb'
SML_CSHARP_ALIAS = 'sml_csharp'
SML_JAVA_ALIAS = 'sml_java'
SML_PYTHON_ALIAS = 'sml_python'
SML_TCL_ALIAS = 'sml_tcl'

DEF_OUT = 'out'
DEF_BUILD = 'build'
DEF_TARGETS = ['kernel', 'cli', SML_JAVA_ALIAS, 'debugger', 'headers', 'scripts', COMPILE_DB_ALIAS]

print("================================================================================")
print(f"Building Soar{SOAR_VERSION}              * will be built if no target specified")
print("Targets available:")
print("   Core:              kernel* cli* scripts*")
print("   Testing:           performance_tests tests")
print(f"   SWIG:              {SML_PYTHON_ALIAS} {SML_TCL_ALIAS} {SML_JAVA_ALIAS}* {SML_CSHARP_ALIAS}")
print(f"   Extras:            debugger* headers* {COMPILE_DB_ALIAS}* tclsoarlib {MSVS_ALIAS} list")
print("Custom Settings available:                                              *default")
print("   Build Type:        --dbg, --opt*, --static")
print("   Custom Paths:      --out, --build, --tcl, --tcl-suffix")
print("   Compilation time:  --no-svs, --scu*, --no-scu, --no-scu-kernel, --no-scu-cli")
print("   Customizations:    --cc, --cxx, --cflags, --lnflags, --no-default-flags, --verbose,")
print("Supported platforms are 64-bit Windows, Linux, and macOS (Intel and ARM)")
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

Export('InstallDir')

AddOption('--cc', action='store', type='string', dest='cc', nargs=1, metavar='COMPILER', help='Use argument as the C compiler.')
AddOption('--cxx', action='store', type='string', dest='cxx', nargs=1, metavar='COMPILER', help='Use argument as the C++ compiler.')
AddOption('--cflags', action='store', type='string', dest='cflags', nargs=1, help='Compiler flags')
AddOption('--lnflags', action='store', type='string', dest='lnflags', nargs=1, help='Linker flags')
AddOption('--no-default-flags', action='store_false', dest='defflags', default=True, help="Don't pass any default flags to the compiler or linker")
AddOption('--no-scu', action='store_false', dest='scu', default=False, help='Don\'t build using single compilation units.')
AddOption('--no-scu-kernel', action='store_true', dest='no_scu_kernel', default=False, help='Never build kernel in a single compilation unit.')
AddOption('--no-scu-cli', action='store_true', dest='no_scu_cli', default=False, help='Never build CLI in a single compilation unit.')
AddOption('--scu', action='store_true', dest='scu', default=True, help='Build using single compilation units.')
AddOption('--out', action='store', type='string', dest='outdir', default=DEF_OUT, nargs=1, metavar='DIR', help='Directory to install binaries. Defaults to "out".')
AddOption('--build', action='store', type='string', dest='build-dir', default=DEF_BUILD, nargs=1, metavar='DIR', help='Directory to store intermediate (object) files. Defaults to "build".')
AddOption('--python', action='store', type='string', dest='python', default=sys.executable, nargs=1, help='Python executable; defaults to same executable used to run SCons')
AddOption('--tcl', action='store', type='string', dest='tcl', nargs=1, help='Path to Tcl installation (ActiveTcl or otherwise)')
AddOption('--tcl-suffix', action='store', type='string', dest='tcl_suffix', default="t", nargs=1, help='Tcl binary suffix (defaults to "t", which is used to indicate full threading support in the standard Tcl build)')
AddOption('--static', action='store_true', dest='static', default=False, help='Use static linking')
AddOption('--dbg', action='store_true', dest='dbg', default=False, help='Enable debug build.  Disables compiler optimizations, includes debugging symbols, debug trace statements and assertions')
AddOption('--opt', action='store_false', dest='dbg', default=False, help='Enable optimized build.  Enables compiler optimizations, removes debugging symbols, debug trace statements and assertions')
AddOption('--verbose', action='store_true', dest='verbose', default=False, help='Output full compiler commands')
AddOption('--no-svs', action='store_true', dest='nosvs', default=False, help='Build Soar without SVS functionality')

if enscons_active:
    tools = ['default', 'packaging', enscons.generate]
else:
    tools = None

env = Environment(
    tools=tools,
    ENSCONS_ACTIVE=enscons_active,
    ENV=os.environ.copy(),
    SCU=GetOption('scu'),
    DEBUG=GetOption('dbg'),
    NO_SCU_KERNEL=GetOption('no_scu_kernel'),
    NO_SCU_CLI=GetOption('no_scu_cli'),
    BUILD_DIR=GetOption('build-dir'),
    OUT_DIR=os.path.realpath(GetOption('outdir')),
    TCL_PATH = GetOption('tcl'),
    TCL_SUFFIX = GetOption('tcl_suffix'),
    # We fail the build immediately if Tcl cannot be loaded but was specifically requested.
    # This is done because the Tcl build is super fragile and it's easy to accidentally
    # build without it.
    TCL_REQUIRED = 'tclsoarlib' in COMMAND_LINE_TARGETS or 'sml_tcl' in COMMAND_LINE_TARGETS,
    SOAR_VERSION=SOAR_VERSION,
    VISHIDDEN=False,  # needed by swig
	JAVAVERSION='11.0',
    SML_CSHARP_ALIAS = SML_CSHARP_ALIAS,
    SML_JAVA_ALIAS = SML_JAVA_ALIAS,
    SML_PYTHON_ALIAS = SML_PYTHON_ALIAS,
    SML_TCL_ALIAS = SML_TCL_ALIAS,
    # indentation for log formatting
    INDENT = '    ',
    # used for generating the MSVS project
    SCONS_HOME=os.path.join(script_dir, 'scons', 'scons-local-4.4.0')
)

env.AddMethod(prepare_for_compiling_with_tcl, 'PrepareForCompilingWithTcl')

# must be specified first or else the resulting file will not contain all compile commands
env.Tool('compilation_db')
compile_db_target = env.CompilationDatabase()
env.Alias(COMPILE_DB_ALIAS, compile_db_target)

# This creates a file for cli_version.cpp to source.  For optimized builds, this guarantees
# that the build date will be correct in every build.  (Turned off for debug, b/c it was adding
# extra compilation time.  (for some reason, this will build it the first two times you compile after
# it exists.)

if ((env['DEBUG'] == None) or (env['DEBUG'] == False) or (FindFile('build_time_date.h', 'Core/shared/') == None)):
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

# TODO: Enabling all the warnings is a WIP! These are very thorough and need to be disabled for
# parts we don't control, like the SWIG-generated code.
if compiler == "msvc":
    pass
    # show all warnings
    # cflags.append('/W4')
    # treat warnings as errors
    # cflags.append('/WX')
else:
    # show all warnings
    # cflags.extend(['-Wall'])
    # treat warnings as errors
    # cflags.extend(['-Werror'])

    # We're starting with something simple. We'll add more as we go.
    cflags.extend(['-Wunused-variable', '-Wreorder'])

    # warning doesn't exist in Apple's clang
    if sys.platform != 'darwin':
        # causes some spurious warnings; TODO: revisit and re-enable if newer compiler version fixes that
        cflags.append('-Wno-stringop-overflow')

if compiler == 'g++':
    libs += [ 'pthread', 'dl', 'm' ]
    if GetOption('nosvs'):
        cflags.append('-DNO_SVS')
    if GetOption('defflags'):
        if env['DEBUG']:
            cflags.extend(['-g'])
        else:
            cflags.extend(['-O3', '-DNDEBUG'])

        gcc_ver = gcc_version(env['CXX'])
        # check if the compiler supports -fvisibility=hidden (GCC >= 4)
        # If so, hide symbols not explicitly marked for exporting using our EXPORT macro
        if gcc_ver[0] > 3:
            env.Append(CPPFLAGS='-fvisibility=hidden')

            config = Configure(env)
            if config.TryCompile('', '.cpp'):
                cflags.append('-fvisibility=hidden')
                # TODO: what does this do?
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
    cflags.extend(['/EHsc', '/D', '_CRT_SECURE_NO_DEPRECATE', '/D', '_WIN32', '/bigobj'])
    if GetOption('nosvs'):
        cflags.extend(' /D NO_SVS'.split())
    if GetOption('defflags'):
        if env['DEBUG']:
            cflags.extend(' /MDd /Zi /Od /DEBUG'.split())
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

if sys.platform == 'win32':
    env.Append(CXXFLAGS=f'/std:{CPP_STD_VERSION}')
else:
    env.Append(CXXFLAGS=f'-std={CPP_STD_VERSION}')

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

    if env['DEBUG']:
        g_msvs_variant = 'Debug|x64'
    else:
        g_msvs_variant = 'Release|x64'

Export('g_msvs_variant')

for d in os.listdir('.'):
    if not d.startswith('.'):
        script = join(d, 'SConscript')
        if os.path.exists(script):
            SConscript(script, variant_dir=join(GetOption('build-dir'), d), duplicate=0)

# section Python-related packaging
Import('python_shlib')
Import('python_source')
Import('soarlib')
py_lib_namespace = "soar_sml"

# Targets to be built and included in wheel files.
py_sources = []
# Targets to be built but NOT included in wheel files.
py_extra = []

if sys.platform == 'darwin' or os.name == 'nt':
    # Add soar's library to the wheel directory.
    #
    # With MacOS builds, these are shipped in the final wheel archive. Ditto for Windows.
    #
    # With linux builds, this step isn't neccecary, as its linker will pick up on the library from out/,
    # and statically link it against the SWIG-generated shared library.
    py_sources += [
        env.Install(py_lib_namespace, soarlib)
    ]

if sys.platform == 'darwin':
    # For MacOS, also add to the out/ directory,
    # so the linker can pick up on it properly.
    py_extra += [
        env.Install(env['OUT_DIR'], soarlib)
    ]

py_sources += [
    env.Install(py_lib_namespace, python_shlib),
    env.InstallAs(py_lib_namespace + "/__init__.py", python_source)
]

env.Alias(SML_PYTHON_ALIAS + "_dev", py_sources)

if enscons_active:
    # Instead of giving an explicit tag, we tell enscons that we're not building a "pure" (python-only) library,
    # and so we let it determine the wheel tag by itself.
    env['ROOT_IS_PURELIB'] = False

    # Whl and WhlFile add multiple targets (sdist, dist_info, bdist_wheel, editable) to env
    # for enscons (python build backend for scons; required for building with cibuildwheel).
    whl = env.Whl("platlib", py_sources, root="")

    # Adding Depends makes scons build this file, but enscons will not include it in the final wheel file.
    env.Depends(whl, py_extra)

    env.WhlFile(source=whl)

    sdist_sources = [
        # Add SCons related files
        "SConstruct",
        "Core/SConscript",
        *env.Glob("Core/*/SConscript"),
        "Core/ClientSMLSWIG/Python/SConscript",

        # Add SWIG files
        *env.Glob("Core/ClientSMLSWIG/*.i"),
        *env.Glob("Core/ClientSMLSWIG/Python/*.i"),

        # Add build support files
        *env.Glob("build_support/*.py"),

        # Entire SVS source tree
        "Core/SVS",

        # Misc files
        "Core/ClientSMLSWIG/Python/README.md",
    ]

    # Look for files under Core with the following file extensions, for up to 5 levels deep.
    #
    # We cannot just add the directories, as they will then start to include the .tar.gz file,
    # which scons treats as a "target", creating a circular dependency.
    SOURCE_EXTS = {"h", "c", "cpp", "hpp", "cxx"}
    for i in range(0, 5):
        for ext in SOURCE_EXTS:
            sdist_sources.extend(
                env.Glob("Core/" + ("*/" * i) + "*." + ext)
            )

    env.SDist(
        source=sdist_sources,
        # We tell enscons to include a generated / corrected pyproject.toml into the source distribution
        pyproject=True,
    )

    # We make sure that an editable (`pip install -e`) installation always properly installs
    # the files in the correct places.
    env.Depends("editable", py_sources)

# endsection Python-related packaging

if 'MSVSSolution' in env['BUILDERS']:

    msvs_solution = env.MSVSSolution(
        target='soar' + env['MSVSSOLUTIONSUFFIX'],
        projects=msvs_projs,
        variant=g_msvs_variant,
    )

    env.Alias(MSVS_ALIAS, [msvs_solution] + msvs_projs)

ALL_ALIAS = 'all'
all_aliases = list(default_ans.keys())
env.Alias(ALL_ALIAS, all_aliases)

if COMMAND_LINE_TARGETS == ['list']:
    print('\n'.join(sorted(all_aliases)))
    Exit()

# Set default targets
for a in DEF_TARGETS:
    if a in all_aliases:
        Default(a)

