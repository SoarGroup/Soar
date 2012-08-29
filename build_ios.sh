#!/bin/bash

for var in $@
do
if [ "$var" == "--simulator" ]; then
	PLATFORM="Simulator"
	ARCH="i386"
elif [ "$var" == "--armv6" ]; then
	PLATFORM="OS"
	ARCH="armv6"
elif [ "$var" == "--armv7" ]; then
	PLATFORM="OS"
	ARCH="armv7"
fi
done

if [ -z "$PLATFORM" ]; then
	PLATFORM="Simulator"
	echo "WARNING: Using Simulator for the platform since it was unspecified"
fi

if [ -z "$ARCH" ]; then
	ARCH="i386"
	echo "WARNING: Using i386 for the architectuer since it was unspecified"
fi

if [ -z "$DEVELOPER_HOME" ]; then
	DEVELOPER_HOME="/Applications/Xcode.app/Contents/Developer"
	echo "WARNING: Using Xcode home of /Applications/Xcode.app/Contents/Developer"
fi

if [ -z "$SDKVER" ]; then
	SDKVER="5.1"
	echo "WARNING: Using default SDK version 5.1"
fi

if [ -z "$SDKMIN" ]; then
	SDKMIN="4.2"
	echo "WARNING: Using default SDK min version of 4.2"
fi

PLATFORMPATH="$DEVELOPER_HOME/Platforms/iPhone$PLATFORM.platform"
SDK="$PLATFORMPATH/Developer/SDKs/iPhone$PLATFORM$SDKVER.sdk"

CPPFLAGS="-arch $ARCH -isysroot $SDK -miphoneos-version-min=$SDKMIN"
LINKFLAGS="$CPPFLAGS"

if [ "$PLATFORM" != "Simulator" ]; then
	CPPFLAGS="$CPPFLAGS -DIPHONE_SDK -D__LLP64__"
else
	CPPFLAGS="$CPPFLAGS -m32"
fi

CC="clang"
CXX="clang++"

SCONSFLAGS="--cc='$CC' --cxx='$CXX' --lnflags='$LINKFLAGS' --cflags='$CPPFLAGS' --out='out-$ARCH' --build='build-$ARCH' --static"

if [ "$ARCH" != "i386" ]; then
	SCONSFLAGS="$SCONSFLAGS --opt"
fi

bash -c "scons $SCONSFLAGS kernel"
