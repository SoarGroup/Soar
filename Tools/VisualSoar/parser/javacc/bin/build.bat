
set MBACKUP=%CLASSPATH%
set CLASSPATH=C:\Bob\soar dev\SoarSuite\Tools\VisualSoar\parser\javacc\JavaCC.zip
java COM.sun.labs.javacc.Main ..\..\soarparser.jj
set CLASSPATH=%MBACKUP%

copy *.java ..\..\..\Source\edu\umich\visualsoar\parser
cd ..\..\..\
build.bat
pause