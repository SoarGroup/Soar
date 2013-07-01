import sys, os
import subprocess as sub
import itertools as itl

def CheckSWIG(env):
	if not env.WhereIs('swig'):
		return False
	
	p = sub.Popen('swig -version'.split(), stdout=sub.PIPE)
	out = p.communicate()[0].split()
	p.wait()
	
	version = tuple(int(x) for x in out[2].split('.'))
	if version >= (1, 3, 31):
		return True
	
	print 'swig version 1.3.31 or higher is required'
	return False

Import('env', 'compiler', 'InstallDir', 'g_msvs_variant')
kernel_env = env.Clone()

# Windows DLLs need to get linked to dependencies, whereas Linux and Mac shared objects do not
if os.name == 'nt':
	kernel_env['LIBS'] = ['advapi32']
else:
	kernel_env['LIBS'] = []

if sys.platform == 'darwin':
	install_name = os.path.join('@loader_path', env['LIBPREFIX'] + 'Soar' + env['SHLIBSUFFIX'])
	kernel_env.Append(LINKFLAGS = ['-install_name', install_name])

if env['SCU']:
	scu = 0
else:
	scu = 1

srcs = [
    # SCU source                        Non-SCU source
	('SoarKernel/SoarKernel.cxx',       Glob('SoarKernel/src/*.cpp')),    # Kernel
	('SoarKernel/sqlite/sqlite3.c',     ['SoarKernel/sqlite/sqlite3.c']), # sqlite
	('KernelSML/KernelSML.cxx',         Glob('KernelSML/src/*.cpp')),     # KernelSML
	('ClientSML/ClientSML.cxx',         Glob('ClientSML/src/*.cpp')),     # ClientSML
	('ConnectionSML/ConnectionSML.cxx', Glob('ConnectionSML/src/*.cpp')), # ConnectionSML
	('ElementXML/ElementXML.cxx',       Glob('ElementXML/src/*.cpp')),    # ElementXML
	('CLI/CommandLineInterface.cxx',    Glob('CLI/src/*.cpp')),           # CLI
]

if compiler == 'msvc':
	srcs.append(('pcre/pcre.cxx', Glob('pcre/*.c')))

if GetOption('static'):
	soarlib = kernel_env.Library('Soar', [s[scu] for s in srcs])
else:
	soarlib = kernel_env.SharedLibrary('Soar', [s[scu] for s in srcs])[:2]
	if compiler == 'msvc':
		kernel_env.Append(CPPFLAGS = ['/D', '_USRDLL'])

lib_install = env.Alias('kernel', env.Install('$OUT_DIR', soarlib))

# headers
headers = []
for d in ['ElementXML/src', 'ConnectionSML/src', 'ClientSML/src', 'shared']:
	headers.extend(InstallDir(env, '$OUT_DIR/include', env.Dir(d).srcnode().abspath, '*.h*'))

env.Alias('headers', headers)

if not CheckSWIG(env):
	print 'swig not found, will not attempt to build wrappers'
else:
	for x in 'Python Java Tcl PHP CSharp'.split():
		SConscript(os.path.join('ClientSMLSWIG', x, 'SConscript'))

if 'MSVSProject' in kernel_env['BUILDERS']:
	vcproj = kernel_env.MSVSProject(
		target = '#core' + env['MSVSPROJECTSUFFIX'],
		srcs = list(itl.chain.from_iterable([[str(f) for f in s[1]] for s in srcs])),
		buildtarget = lib_install,
		variant = g_msvs_variant,
		auto_build_solution = 0,
	)
	
	Import('msvs_projs')
	msvs_projs.append(vcproj)
