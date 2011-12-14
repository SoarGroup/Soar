#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>
#
import glob
import itertools as itl

Import('env')
clone = env.Clone()
clone.Prepend(CPPPATH = Split('src src/filters src/models #SoarKernel/src'))

src = list(itl.chain(*[Glob(p) for p in ('src/*.cpp', 
                                         'src/filters/*.cpp', 
                                         'src/commands/*.cpp', 
                                         'src/models/*.cpp')]))

# I want to build nn.cpp separately using some unsafe flags
src = filter(lambda x: not x.path.endswith('nn.cpp'), src)
#if not any('filter_factory.cpp' in str(f) for f in src):
#	src.append('#SVS/src/filter_factory.cpp')

#Command('#SVS/src/filter_factory.cpp', Glob('src/filters/*.cpp'), './gen_filter_factory', chdir='SVS')

nnobj = clone.Object('src/models/nn.cpp', CPPFLAGS = '-O3 -ffast-math -ftree-vectorizer-verbose=2 -march=native')
svs = clone.StaticLibrary('svs', src + [nnobj])
env.Install('$PREFIX/lib', svs)

