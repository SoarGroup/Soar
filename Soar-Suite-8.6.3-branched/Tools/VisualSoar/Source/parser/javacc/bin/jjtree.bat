@echo off
set MBACKUP=%CLASSPATH%
set CLASSPATH=C:\Program Files\JavaCC\javacc\JavaCC.zip
java COM.sun.labs.jjtree.Main %1 %2 %3 %4 %5 %6 %7 %8 %9
set CLASSPATH=%MBACKUP%
