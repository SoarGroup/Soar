#!/bin/bash

# This is meant to be run from the parent directory (containing SoarKernel,
# gSKI, etc...)

TLD=`pwd`;

if test ! -d "SoarKernel"; then
  echo "SoarKernel not found!";
  exit 1;
fi

cd ${TLD}/SoarKernel; 

if [[ ./configure ]]
 make
cd ${TLD}/gSKI; ./configure && make
cd ${TLD}/SoarIO; ./configure --prefix=${HOME}/sandbox && make && make install
cd ${TLD}/SoarIO/examples/TestJavaSML; buildJava.sh
# cd ${TLD}/SoarJavaDebugger;
