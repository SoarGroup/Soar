#!/bin/bash

for var in $@
do
	if [ "$var" == "--phone" ]; then
		PLATFORM="OS"
	elif [ "$var" == "--simulator" ]; then
		PLATFORM="Simulator"
	fi
done

if [ -z "$PLATFORM" ]; then
	PLATFORM="Simulator"
	echo "WARNING: Using Simulator for the platform since it was unspecified"
fi

if [ -z "$DEVELOPER_HOME" ]; then
	DEVELOPER_HOME="/Applications/Xcode.app/Contents/Developer"
	echo "WARNING: Using Xcode home of /Applications/Xcode.app/Contents/Developer"
fi

if [ -z "$SDKVER" ]; then
	SDKVER="7.0"
	echo "WARNING: Using default SDK version 7.0"
fi

if [ -z "$SDKMIN" ]; then
	SDKMIN="7.0"
	echo "WARNING: Using default SDK min version of 7.0"
fi

PLATFORMPATH="$DEVELOPER_HOME/Platforms/iPhone$PLATFORM.platform"
SDK="$PLATFORMPATH/Developer/SDKs/iPhone$PLATFORM$SDKVER.sdk"

if [ "$PLATFORM" == "Simulator" ]; then
	CPPFLAGS="-arch i386 -arch x86_64 -isysroot $SDK -miphoneos-version-min=$SDKMIN"
elif [ "$PLATFORM" == "OS" ]; then
	CPPFLAGS="-arch arm64 -arch armv7 -arch armv7s -isysroot $SDK -miphoneos-version-min=$SDKMIN -D__LLP64__ -DIPHONE_SDK"
fi

LINKFLAGS="$CPPFLAGS"

command -v ccache >/dev/null

if [ $? -eq 0 ]; then
	CC="ccache"
	CXX="ccache"
fi

CC="$CC clang -Qunused-arguments"
CXX="$CXX clang++ -Qunused-arguments"

if [ "$PLATFORM" == "Simulator" ]; then
	scons --cc="$CC" --cxx="$CXX" --lnflags="$LINKFLAGS" --cflags="$CPPFLAGS" --out="out-simulator" --build="build-simulator" --static kernel
elif [ "$PLATFORM" == "OS" ]; then
	scons --cc="$CC" --cxx="$CXX" --lnflags="$LINKFLAGS" --cflags="$CPPFLAGS" --out="out-iphone" --build="build-iphone" --static kernel
fi
