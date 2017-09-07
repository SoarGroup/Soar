#!/bin/bash

CCACHE=$(which ccache)
if [ ! -x $CCACHE ]; then
  CCACHE=""
fi

export CCACHE
export CCACHE_BASEDIR=$HOME
export CCACHE_CC=/usr/bin/clang
export CCACHE_CXX=/usr/bin/clang++
export CCACHE_SLOPPINESS=time_macros
export CCACHE_COMPILERCHECK="$CCACHE_CC -v"
export BUILD=build_cd
export OUT=out_cd

user_args='$OUT/libSoar.so $OUT/libJava_sml_ClientInterface.so $OUT/SoarJavaDebugger.jar'
lsb_args='--cc="$CCACHE $CCACHE_CC" \
  --cxx="$CCACHE $CCACHE_CXX" \
  --build="$BUILD" \
  --out="$OUT"'

# need to use eval here so that quoting in $args is resolved correctly
eval "scons $user_args $lsb_args"
rv=$?

if [ $rv -ne 0 ]; then
  exit 125
fi

echo "Is this revision good/bad?"
select goodbad in "good" "bad"; do
  case $goodbad in
    good ) exit 0;;
    bad ) exit 1;;
  esac
done
