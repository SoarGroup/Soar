#!/bin/bash

CC=/usr/bin/x86_64-linux-gnu-gcc-4.4
LSB_CC=/opt/lsb/bin/lsbcc
LSBCC_LSBVERSION=4.0
LSBCC_LIB_PREFIX=/opt/lsb/lib64-
LSBCC_LIBS=$LSBCC_LIB_PREFIX$LSBCC_LSBVERSION

CCACHE_BASEDIR=$HOME \
CCACHE_SLOPPINESS=time_macros \
CCACHE_COMPILERCHECK="$LSB_CC --lsb-cc=$CC -v" \
ccache \
  $LSB_CC \
    --lsb-target-version=$LSBCC_LSBVERSION \
    --lsb-libpath=$LSBCC_LIBS \
    --lsb-cc=$CC \
    "$@"
