@rem
@rem Run with make-all-windows <version> where e.g. <version> = 020
@rem Results are copied into Releases folder.
@rem

@rem Save the initial value of some env vars
@set svpath=%path%
@set svinc=%include%
@set svlib=%lib%

@rem Check if a version was provided
@rem @IF "%1" == "" goto Help1
@rem @set ver=%1%

@rem Set up the ROOT, VC6 and JDK env vars if needed
call windows-env.bat

@IF "%VC6%" == "" goto help2
@IF "%JDK%" == "" goto help2
@IF "%RELEASES%" == "" goto help2
@rem @IF "%TCL%" == "" goto help2

@rem Set debug/release configuration here
set config=Release

@rem Set the soar version (used to find the soar files)
@rem @set soarver=Soar-8.3

@rem Create the output directory (removing the existing one if needed)
@rem @set output=%RELEASES%\V%ver%-Windows
@rem rmdir /s /q "%OUTPUT%"
@rem mkdir "%OUTPUT%"

@rem Set up the include path etc. for VC++ 6
@call "%VC6%\vc98\bin\vcvars32.bat"
@echo on

@rem Set up the include path etc. for JDK
@set PATH=%JDK%\bin;%PATH%

@rem Set up the include path etc. for TCL
@rem @set INCLUDE=%TCL%\include;%INCLUDE%

@echo Build STI
@rem msdev "STI\STI.dsw" /MAKE "TestMaster - Win32 %config%" /REBUILD /out "%OUTPUT%\STIBuild.txt"
msdev "STI\STI.dsw" /MAKE "TestMaster - Win32 %config%" /REBUILD /out STIBuild.txt"

@rem @echo Build Soar-8.5.1
@rem msdev "%soarver%\interface\SoarInterface.dsw" /MAKE "dll - Win32 %config%" /REBUILD /out "%OUTPUT%\SoarBuild.txt"

@echo Build VisualSoar
@rem @cd "VisualSoar\Source"
@cd "Source"
@del edu\umich\visualsoar\*.class
javac edu\umich\visualsoar\VisualSoar.java

@echo Build VisualSoar jar file
@del ..\visualsoar.jar
@rem *** BADBAD -- This collects too much at the moment ***
@rem jar cvfm VisualSoar.jar meta-inf\manifest.mf -C . * > ..\..\%OUTPUT%\VSBuild.txt
jar cvfm VisualSoar.jar meta-inf\manifest.mf -C . * > ..\VSBuild.txt
@cd ..

@echo Copy results to release point

@rem For now, copy existing release from Soar and modify it.
@rem Later we should be able to build this directly from the source files.
@rem @xcopy /s /q "windowsBase\%soarver%\*.*" "%OUTPUT%\%soarver%\*.*"
@rem @xcopy /s /q "WindowsBase\vs\*.*" "%OUTPUT%\vs\*.*"

@rem Overwrite the relevant parts
@rem @xcopy /Y /q "%soarver%\interface\*.dll" "%OUTPUT%\%soarver%\interface\*.dll"
@rem @xcopy /Y /q "%soarver%\interface\*.dll" "%OUTPUT%\%soarver%\library\*.dll"
@rem @xcopy /Y /q "%soarver%\library\tsi31" "%OUTPUT%\%soarver%\library\tsi31\"
@rem @xcopy /Y /q "visualsoar\Source\VisualSoar.jar" "%OUTPUT%\vs\visualSoar.jar"
@move /Y "Source\VisualSoar.jar"

@rem Copy over STI files
@rem @xcopy /q "STI\%config%\*.dll" "%OUTPUT%\STI\*.dll"
@rem @xcopy /q "STI\%config%\*.exe" "%OUTPUT%\STI\*.exe"

@rem For Windows, copy STI DLLs into position where exe's will file them automatically
@rem @xcopy /q "STI\%config%\libSTI*.dll" "%OUTPUT%\%soarver%\*.*"
@rem @xcopy /q "STI\%config%\*.dll" "%OUTPUT%\vs\*.dll"
@move /Y "STI\%config%\*.dll"

@echo Build completed

@goto done

@rem :help1
@rem @echo Syntax is "make-all-windows <version> where e.g. <version> = 020
@rem goto done

:help2
@echo Need to set VC6, JDK, TCL and RELEASES in windows-env.bat
goto done

:done
@echo on
@set path=%svpath%
@set include=%svinc%
@set lib=%svlib%
