#!/bin/sh

echo SoarKernel
cd SoarKernel
cvs -q up -dP
echo gSKI
cd ../gSKI
cvs -q up -dP
echo SoarIO
cd ../SoarIO
cvs -q up -dP
echo SoarLibrary
cd ../soar-library
cvs -q up -dP
echo Documentation
cd ../Documentation
cvs -q up -dP
echo JavaMissionaries
cd ../JavaMissionaries
cvs -q up -dP
echo JavaTOH
cd ../JavaTOH
cvs -q up -dP
echo SoarJavaDebugger
cd ../SoarJavaDebugger
cvs -q up -dP
echo tclenvironments
cd ../tclenvironments
cvs -q up -dP
