#!/bin/bash

check_output() {
	unset LSB_SHAREDLIBPATH
	appchecker="$LSB_HOME/bin/lsbappchk"
	if [ ! -x "$appchecker" ]
	then
		echo appchecker not found >&2
		exit 1
	fi

	if [ ! -d "$1" ]
	then
		echo no such output directory >&2
		exit 1
	fi

	files="cli TestCLI TestExternalLibrary TestSMLEvents TestSMLPerformance UnitTests libSoar.so libJava_sml_ClientInterface.so"
	opts="--no-journal --missing-symbols --lsb-version=$LSBCC_LSBVERSION -L $1/libSoar.so"

	for f in $files
	do
		p="$1/$f"
		if [ -f "$p" ]
		then
			echo Checking $p
			$appchecker $opts "$p" &>lsbappchk.log
			if ! grep 'No unspecified symbols were found' lsbappchk.log >/dev/null
			then
				echo $p is not LSB compliant. Details in lsbappchk.log
				exit 1
			fi
			rm lsbappchk.log
		fi
	done
}

gcc_version() {
	$1 --version | awk '$NF !~ /[0-9]\.[0-9]\.[0-9]/ {exit 1} {split($NF,v,"."); print v[1], v[2], v[3]}' || return 1
}

OUT=out
for a in $*
do
	case "$a" in
	--out=*)
		OUT=${a:6}
		;;
	--cc=*)
		CC=${a:5}
		;;
	--cxx=*)
		CXX=${a:6}
		;;
	--lnflags=*)
		LNFLAGS=${a:10}
		;;
	*)
		user_args="$user_args \"$a\""
		;;
	esac
done

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
VERCC=( `gcc_version $CC` )
VERCXX=( `gcc_version $CXX` )
echo "Detected gcc version ${VERCC[0]}.${VERCC[1]}.${VERCC[2]}"
echo "Detected g++ version ${VERCXX[0]}.${VERCXX[1]}.${VERCXX[2]}"

if [ ! -x "$LSB_HOME/bin/lsbcc" ] || [ ! -x "$LSB_HOME/bin/lsbc++" ]; then
  COMPILE_FOR_LSB=0
  echo "LSB compilation toolchain not found."
fi

if [ $COMPILE_FOR_LSB -ne 0 ] && [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt 4 -o ${VERCXX[1]} -gt 4 ]
then
  echo "GCC 4.5 and up require ld.gold for LSB compilation."

  GOLD_LD=$(echo $(whereis -b gold-ld) | sed 's/.* //')
  if [ -x "$GOLD_LD/ld" ]; then
    echo "ld.gold found: $GOLD_LD"
  else
    echo "gold-ld could not be found."
    echo "Notes: gold-ld is merely a directory on the path (e.g. /usr/lib/gold-ld)"
    echo "       gold-ld must contain 'ld', a symlink to ld.gold or gold"
  fi
fi

#if [ $COMPILE_FOR_LSB -ne 0 ] && [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt $MAX_MINOR -o ${VERCXX[1]} -gt $MAX_MINOR ]
#then
#  COMPILE_FOR_LSB=0
#  for minor in $(seq $MAX_MINOR -1 0); do
#    TESTCC=$(which "gcc-4.$minor")
#    TESTCXX=$(which "g++-4.$minor")
#    if [ "$TESTCC" != "" ] && [ "$TESTCXX" != "" ] && [ -x $TESTCC -a -x $TESTCXX ]; then
#      COMPILE_FOR_LSB=1
#      CC=$TESTCC
#      CXX=$TESTCXX
#      VERCC[0]=4
#      VERCXX[0]=4
#      VERCC[1]=$MAX_MINOR
#      VERCXX[1]=$MAX_MINOR
#      break
#    fi
#  done
#
#  if [ $COMPILE_FOR_LSB -eq 0 ]; then
#    echo "No version of gcc/g++ usable for LSB compilation found."
#  fi
#fi

if [ $COMPILE_FOR_LSB -eq 0 ]; then
  exit 1
fi

export LSBCC_LIB_PREFIX
export LSBCC=$CC
export LSBCXX=$CXX
export LSBCC_LSBVERSION=4.1
export LSBCC_LIBS=$LSBCC_LIB_PREFIX$LSBCC_LSBVERSION
export LSBCC_SHAREDLIBS=python2.7:Soar
export CCACHE
export CCACHE_BASEDIR=$HOME
export CCACHE_CC=$LSB_HOME/bin/lsbcc
export CCACHE_CXX=$LSB_HOME/bin/lsbc++
export CCACHE_SLOPPINESS=time_macros
export CCACHE_COMPILERCHECK="$CCACHE_CC -v"

if [ ${VERCC[0]} -gt 4 -o ${VERCXX[0]} -gt 4 -o ${VERCC[1]} -gt 4 -o ${VERCXX[1]} -gt 4 ]; then
  echo "Using ld.gold: $GOLD_LD"
  LNFLAGS="$LNFLAGS -B$GOLD_LD -Wl,--no-gnu-unique"
fi

lsb_args='--cc="$CCACHE $CCACHE_CC --lsb-cc=$LSBCC" \
  --cxx="$CCACHE $CCACHE_CXX --lsb-cxx=$LSBCXX" \
  --lnflags="$LNFLAGS --lsb-shared-libs=$LSBCC_SHAREDLIBS -Wl,--hash-style=both" \
  --out="$OUT"'

# need to use eval here so that quoting in $args is resolved correctly
eval "python scons/scons.py $user_args $lsb_args" || exit 1
check_output $OUT

