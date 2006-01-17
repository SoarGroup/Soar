#!/usr/bin/python

##
# This script generates a simple shell script to run
# soardoc. It must be run from the directory it's in!
#
import os

exeName = os.sys.executable
curDir = os.getcwd()
outFileName = ''
if os.sys.platform == 'win32':
   outFileName = 'soardoc.bat'
   f = open(outFileName, 'w')
   f.write('echo off\n')
   f.write('set PYTHON_EXE="%s"\n' % exeName)
   f.write('set SOARDOC_EXE="%s\\src\\soardoc.py"\n' % curDir)
   f.write('%PYTHON_EXE% %SOARDOC_EXE% %*\n')
else:
   outFileName = 'soardoc'
   f = open(outFileName, 'w')
   f.write('#!/bin/sh\n')
   f.write('PYTHON_EXE="%s"\n' % exeName)
   f.write('SOARDOC_EXE="%s/src/soardoc.py"\n' % curDir)
   f.write('$PYTHON_EXE $SOARDOC_EXE $*\n')
   os.chmod(outFileName, 0755)

print 'Created shell script %s.' % outFileName
print 'Make sure that "%s" is on your system path.' % curDir
