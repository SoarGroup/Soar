#!/usr/bin/python
# Project: Soar <http://soar.googlecode.com>
# Author: Jonathan Voigt <voigtjr@gmail.com>
#
Import('compEnv')

SoarTextIO = compEnv.Program('SoarTextIO', Glob('src/*.cpp'))
compEnv.Install('$PREFIX/bin', SoarTextIO)

