@echo off
cd SoarSuite\Environments\Soar2D

set SOARLIB=../../SoarLibrary/bin
set PATH=%SOARLIB%;%PATH%

start javaw -jar Soar2D.jar eaters.xml
