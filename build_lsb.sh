#!/bin/bash

if [ "$LSB_HOME" == "" ]; then
	LSB_HOME=/opt/lsb
fi

CCACHE=""
if [ -x $(which ccache) ]; then
	CCACHE=ccache
fi

LSBCC_LIB_PREFIX=$LSB_HOME/lib-
if [ $(uname -m) == "x86_64" ]; then
	LSBCC_LIB_PREFIX=$LSB_HOME/lib64-
fi

export LSBCC_LIB_PREFIX
export LSBCC=gcc-4.6
export LSBCXX=g++-4.6
export LSBCC_LSBVERSION=4.0
export LSBCC_LIBS=$LSBCC_LIB_PREFIX$LSBCC_LSBVERSION
export CCACHE
export CCACHE_BASEDIR=$HOME
export CCACHE_CC=$LSB_HOME/bin/lsbcc
export CCACHE_CXX=$LSB_HOME/bin/lsbc++
export CCACHE_SLOPPINESS=time_macros
export CCACHE_COMPILERCHECK="$CCACHE_CC -v"

mkdir -p out

scons \
	--cc="$CCACHE $CCACHE_CC --lsb-cc=$LSBCC" \
	--cxx="$CCACHE $CCACHE_CXX --lsb-cxx=$LSBCXX" \
	--lnflags="--lsb-shared-libpath=out -Wl,--hash-style=both" \
	cli debugger debugger_api headers java_sml_misc kernel sml_java tests "$@"

$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/cli
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestCLI
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestExternalLibrary
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLEvents
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSMLPerformance
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/TestSoarPerformance
$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=4.0 --shared-libpath=out out/UnitTests
