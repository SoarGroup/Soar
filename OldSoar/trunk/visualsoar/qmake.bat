@echo off

REM create a buffer between last command for easy scroll back
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.
echo.

cd "Source"

echo Deleting old VisualSoar class files
if EXIST edu\umich\visualsoar\VisualSoar.class del /q /s edu\umich\visualsoar\*.class > NUL
if EXIST edu\umich\visualsoar\VisualSoar.class goto badclassdelete
if EXIST edu\umich\visualsoar\datamap\VisualSoar.class del /q /s edu\umich\visualsoar\datamap\*.class > NUL
if EXIST edu\umich\visualsoar\dialogs\VisualSoar.class del /q /s edu\umich\visualsoar\dialogs\*.class > NUL
if EXIST edu\umich\visualsoar\graph\VisualSoar.class del /q /s edu\umich\visualsoar\graph\*.class > NUL
if EXIST edu\umich\visualsoar\misc\VisualSoar.class del /q /s edu\umich\visualsoar\misc\*.class > NUL
if EXIST edu\umich\visualsoar\operatorwindow\VisualSoar.class del /q /s edu\umich\visualsoar\operatorwindow\*.class > NUL
if EXIST edu\umich\visualsoar\parser\VisualSoar.class del /q /s edu\umich\visualsoar\parser\*.class > NUL
if EXIST edu\umich\visualsoar\ruleeditor\VisualSoar.class del /q /s edu\umich\visualsoar\ruleeditor\*.class > NUL
if EXIST edu\umich\visualsoar\util\VisualSoar.class del /q /s edu\umich\visualsoar\util\*.class > NUL

echo Compiling VisualSoar java files
javac -g edu\umich\visualsoar\VisualSoar.java
IF NOT EXIST edu\umich\visualsoar\VisualSoar.class GOTO badjavacompile

echo Building VisualSoar.jar
del /q ..\VisualSoar.jar 2> NUL
if EXIST ..\VisualSoar.jar goto badjardelete1

del /q VisualSoar.jar 2> NUL
if EXIST VisualSoar.jar goto badjardelete2

jar cvfm VisualSoar.jar meta-inf\manifest.mf -C . * > ..\VSBuild.txt
IF NOT EXIST VisualSoar.jar  GOTO badjarcompile
cd ..

echo Copying results to release point

move /Y "Source\VisualSoar.jar"


echo ================ Build completed ============================

goto done

:badclassdelete
cd ..
echo ERROR: Could not delete old class files. Stopping...
goto done


:badjavacompile
cd ..
echo ERROR compiling the VisualSoar.  Stopping...
goto done

:badjarcompile
cd ..
echo ERROR creating VisualSoar.jar.  Stopping...
goto done

:badjardelete1
cd ..
echo ERROR: Could not delete VisualSoar.jar.  Stopping...
goto done

:badjardelete2
cd ..
echo ERROR: Could not delete Source\VisualSoar.jar.  Stopping...
goto done

:done
time /t
