@echo off
setlocal
cd /d %~dp0..
echo === Loading conanvcvars ===
call build\Release\generators\conanvcvars.bat || goto :err
echo === Adding SWIG to PATH ===
set "PATH=C:\swigwin-4.3.1;%PATH%"
where swig || goto :err
echo === cmake --preset Release-swig ===
cmake --preset Release-swig || goto :err
echo === cmake --build --preset Release-swig ===
cmake --build --preset Release-swig || goto :err
echo === DONE ===
exit /b 0
:err
echo BUILD FAILED with errorlevel %errorlevel%
exit /b %errorlevel%
