@echo off

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

echo Deleting VisualSoar.jar
del /q ..\VisualSoar.jar 2> NUL
if EXIST ..\VisualSoar.jar goto badjardelete1

del /q VisualSoar.jar 2> NUL
if EXIST VisualSoar.jar goto badjardelete2

cd ..

echo ================ Clean completed ============================

goto done

:badclassdelete
cd ..
echo ERROR: Could not delete old class files. Stopping...
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
