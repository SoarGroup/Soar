#!/usr/bin/env bash

# This script removes the quarantine attributes from the Soar binaries, which macOS won't let users
# run by default because they are downloaded from the internet.

scriptdir=$( dirname -- "$0"; );

echo "macOS setup: removing quarantine attributes from Soar binaries..."
dirname=${1:-$scriptdir}
for file in "soar" "libSoar.dylib" "libTcl_sml_ClientInterface.dylib" "libtclsoarlib.dylib" "_Python_sml_ClientInterface.so" "libCSharp_sml_ClientInterface.dylib" "sml_csharp.dll" "libJava_sml_ClientInterface.jnilib"; do
  echo "  Removing quarantine attributes from $file..."
  xattr -d com.apple.quarantine "$dirname/$file" >/dev/null 2>&1
done
echo "macOS setup: done."

# if the binaries were not quarantined to begin with, then xattr will return an error code, so we
# explicitly return 0 to ignore this case (the binaries will run without issue)
exit 0
