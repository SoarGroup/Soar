@echo off
cd Environments\JavaBaseEnvironment
call build.bat --nopause
cd ..\JavaEaters
call build.bat --nopause
cd ..\JavaMissionaries
call build.bat --nopause
cd ..\JavaTankSoar
call build.bat --nopause
cd ..\JavaTOH
call build.bat --nopause
cd ..\..\Tools\SoarJavaDebugger
call build.bat --nopause
cd ..\TestJavaSML
call build.bat --nopause
cd ..\VisualSoar
call build.bat --nopause
cd ..\..