@echo off
cd SoarSuite\Environments\JavaTOH

set SOARLIB=../../SoarLibrary/bin
set PATH=%SOARLIB%;%PATH%

start javaw -jar toh.jar
