#!/bin/bash

CXX=/usr/bin/x86_64-linux-gnu-g++-4.4
LSB_CXX=/opt/lsb/bin/lsbc++
LSBCC_LSBVERSION=4.0
LSBCC_LIB_PREFIX=/opt/lsb/lib64-
LSBCC_LIBS=$LSBCC_LIB_PREFIX$LSBCC_LSBVERSION

CCACHE_BASEDIR=$HOME \
CCACHE_SLOPPINESS=time_macros \
CCACHE_COMPILERCHECK="$LSB_CXX --lsb-cxx=$CXX -v" \
ccache \
  $LSB_CXX \
    --lsb-target-version=$LSBCC_LSBVERSION \
    --lsb-libpath=$LSBCC_LIBS \
    --lsb-cxx=$CXX \
    "$@"
