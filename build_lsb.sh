#!/bin/bash

if [ "$LSB_HOME" == "" ]; then
  LSB_HOME=/opt/lsb
fi

CCACHE=$(which ccache)
if [ ! -x $CCACHE ]; then
  CCACHE=""
fi

LSBCC_LIB_PREFIX=$LSB_HOME/lib-
BIT=32
if [ $(uname -m) == "x86_64" ]; then
  LSBCC_LIB_PREFIX=$LSB_HOME/lib64-
  BIT=64
fi

COMPILE_FOR_LSB=1

if [ "$CC" == "" ]; then CC=gcc; fi
if [ "$CXX" == "" ]; then CXX=g++; fi
VERCC=( $($CC --version \
        | grep -o '[0-9]\{1,\}\.[0-9]\{1,\}\.[0-9]\{1,\}' \
        | head -n 1 \
        | sed 's/\([0-9]\{1,\}\)\.\([0-9]\{1,\}\)\.\([0-9]\{1,\}\)/\1 \2 \3/') )
VERCXX=( $($CXX --version \
          | grep -o '[0-9]\{1,\}\.[0-9]\{1,\}\.[0-9]\{1,\}' \
          | head -n 1 \
          | sed 's/\([0-9]\{1,\}\)\.\([0-9]\{1,\}\)\.\([0-9]\{1,\}\)/\1 \2 \3/') )
echo "Detected gcc version ${VERCC[0]}.${VERCC[1]}.${VERCC[2]}"
echo "Detected g++ version ${VERCXX[0]}.${VERCXX[1]}.${VERCXX[2]}"

if [ ! -x "$LSB_HOME/bin/lsbcc" ] || [ ! -x "$LSB_HOME/bin/lsbc++" ]; then
  COMPILE_FOR_LSB=0
  echo "LSB compilation toolchain not found."
fi

if [ $COMPILE_FOR_LSB -ne 0 ] && [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt 4 -o ${VERCXX[1]} -gt 4 ]
then
  echo "gcc/g++ 4.5 and 4.6 require ld.gold for LSB compilation."

  GOLD_LD=$(echo $(whereis -b gold-ld) | sed 's/.* //')
  if [ -x "$GOLD_LD/ld" ]; then
    MAX_MINOR=6
    echo "ld.gold found: $GOLD_LD"
  else
    MAX_MINOR=4
    echo "gold-ld could not be found, but is required for LSB build with GCC 4.5 and 4.6."
    echo "Notes: gold-ld is merely a directory on the path (e.g. /usr/lib/gold-ld)"
    echo "       gold-ld must contain 'ld', a symlink to ld.gold or gold"
  fi
fi

if [ $COMPILE_FOR_LSB -ne 0 ] && [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt $MAX_MINOR -o ${VERCXX[1]} -gt $MAX_MINOR ]
then
  COMPILE_FOR_LSB=0
  for minor in $(seq $MAX_MINOR -1 0); do
    TESTCC=$(which "gcc-4.$minor")
    TESTCXX=$(which "g++-4.$minor")
    if [ -x $TESTCC -a -x $TESTCXX ]; then
      COMPILE_FOR_LSB=1
      CC=$TESTCC
      CXX=$TESTCXX
      VERCC[0]=4
      VERCXX[0]=4
      VERCC[1]=$MAX_MINOR
      VERCXX[1]=$MAX_MINOR
      break
    fi
  done

  if [ $COMPILE_FOR_LSB -eq 0 ]; then
    echo "No version of gcc/g++ usable for LSB compilation found."
  fi
fi

if [ $COMPILE_FOR_LSB -eq 0 ]; then
  exit -1
fi

export LSBCC_LIB_PREFIX
export LSBCC=$CC
export LSBCXX=$CXX
export LSBCC_LSBVERSION=4.1
export LSBCC_LIBS=$LSBCC_LIB_PREFIX$LSBCC_LSBVERSION
export LSB_SHAREDLIBPATH="$(pwd)/out"
export CCACHE
export CCACHE_BASEDIR=$HOME
export CCACHE_CC=$LSB_HOME/bin/lsbcc
export CCACHE_CXX=$LSB_HOME/bin/lsbc++
export CCACHE_SLOPPINESS=time_macros
export CCACHE_COMPILERCHECK="$CCACHE_CC -v"


if [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt 4 -o ${VERCXX[1]} -gt 4 ]; then
  echo "Using ld.gold: $GOLD_LD"
  LDFLAGS="-B$GOLD_LD -Wl,--no-gnu-unique"
fi



mkdir -p out

scons \
  --cc="$CCACHE $CCACHE_CC --lsb-cc=$LSBCC" \
  --cxx="$CCACHE $CCACHE_CXX --lsb-cxx=$LSBCXX" \
  --lnflags="$LDFLAGS --lsb-shared-libpath=out -Wl,--hash-style=both" \
  --opt \
  cli debugger debugger_api headers java_sml_misc kernel sml_java tests tohsml cartpole puddleworld
RV=$?
if [ $RV -ne 0 ]; then
  exit $RV
fi

#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/PuddleWorld
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/CartPole
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/cli
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TestCLI
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TestExternalLibrary
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TestSMLEvents
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TestSMLPerformance
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TestSoarPerformance
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/TOHSML
#$LSB_HOME/bin/lsbappchk --no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION --shared-libpath=out out/UnitTests

cp out/java/sml.jar ../AgentDevelopmentTools/VisualSoar/lib/
pushd ../AgentDevelopmentTools/VisualSoar
ant
cp java/soar-visualsoar-snapshot.jar ../../SoarSuite/out/VisualSoar.jar
popd
