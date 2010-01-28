@echo off
set MBACKUP=%CLASSPATH%
set CLASSPATH=C:\Bob\soar9-dev2\SoarSuite\Tools\VisualSoar\Source\parser\javacc\JavaCC.zip
java COM.sun.labs.javacc.Main %1 %2 %3 %4 %5 %6 %7 %8 %9
set CLASSPATH=%MBACKUP%
