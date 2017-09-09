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
export BUILD=build_c
export OUT=out_c

user_args='--opt puddleworld --opt cli'
lsb_args='--cc="$CCACHE $CCACHE_CC" \
  --cxx="$CCACHE $CCACHE_CXX" \
  --build="$BUILD" \
  --out="$OUT"'

# need to use eval here so that quoting in $args is resolved correctly
eval "scons $user_args $lsb_args" || exit 1

#SOARSUITE=$(pwd)
#cp $OUT/java/sml.jar ../VisualSoar/lib/
#pushd ../VisualSoar
#ant
#cp VisualSoar.jar $SOARSUITE/$OUT/
#popd
