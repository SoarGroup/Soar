Import('env')
inc = [env.Dir(d).srcnode() for d in 'src src/filters src/models src/algorithms eigen bullet/include'.split()]

src = []
for d in ['src', 'src/filters', 'src/commands', 'src/models', 'src/algorithms']:
	src += Glob(d + '/*.cpp')

Return('src', 'inc')
