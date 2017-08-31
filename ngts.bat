call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
call build all --dbg
pause
xcopy /D /R /U /Y .\out\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Tcl\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Core\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Core\ClientSMLSWIG\CSharp\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Core\ClientSMLSWIG\Java\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Core\ClientSMLSWIG\Python\*.* ..\ngts-speech\CSoar
xcopy /D /R /U /Y .\build\Core\ClientSMLSWIG\Tcl\*.* ..\ngts-speech\CSoar
pause

