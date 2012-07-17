Import('env')

test_svs = env.Install('$OUT_DIR', env.Program('test_svs', 'src/progs/test_svs.cpp'))
Default(env.Alias('svs_progs', [test_svs]))

inc = [env.Dir(d).srcnode() for d in 'src src/filters src/models src/algorithms eigen bullet/include'.split()]

src = []
for d in ['src', 'src/filters', 'src/commands', 'src/models', 'src/algorithms']:
	src += Glob(d + '/*.cpp')

Return('src', 'inc')
