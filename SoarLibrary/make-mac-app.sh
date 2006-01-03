#!/bin/sh

# $1 is the name of the app
# $2 is the jar name

APP_PATH="soar-library/$1.app/Contents"

mkdir -p "$APP_PATH/MacOS"
mkdir -p "$APP_PATH/Resources/Java"

cp "soar-library/$1.plist" "$APP_PATH/Info.plist"
cp "soar-library/$2" "$APP_PATH/Resources/Java"
cp "soar-library/java_swt" "$APP_PATH/MacOS"
chmod a+x "$APP_PATH/MacOS/java_swt"
