#!/usr/bin/env bash

# This script removes the quarantine attributes from the Soar binaries, which macOS won't let users
# run by default because they are downloaded from the internet.

scriptdir=$( dirname -- "$0"; );

echo "Removing quarantine attributes from Soar binaries..."
xattr -d com.apple.quarantine "$scriptdir/soar"
xattr -d com.apple.quarantine "$scriptdir/libSoar.dylib"
xattr -d com.apple.quarantine "$scriptdir/libTcl_sml_ClientInterface.dylib"
xattr -d com.apple.quarantine "$scriptdir/libtclsoarlib.dylib"
xattr -d com.apple.quarantine "$scriptdir/_Python_sml_ClientInterface.so"
xattr -d com.apple.quarantine "$scriptdir/libCSharp_sml_ClientInterface.dylib"
xattr -d com.apple.quarantine "$scriptdir/libJava_sml_ClientInterface.jnilib"

echo "Done! You can run Soar normally now."
