#!/bin/bash

CC=/usr/bin/i686-pc-linux-gnu-gcc
LSB_CC=/opt/lsb32/bin/lsbcc
LSBCC_LSBVERSION=4.0
LSBCC_LIB_PREFIX=/opt/lsb32/lib-
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
