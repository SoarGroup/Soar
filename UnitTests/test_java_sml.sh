#!/bin/bash

# Simple test script for Java SWIG bindings
# This script compiles and runs a basic Java test for Soar SML

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Try to detect build directory from environment or common locations
if [ -n "${SOAR_BUILD_DIR}" ]; then
    BUILD_DIR="${SOAR_BUILD_DIR}"
else
    # Try common build directory patterns
    for build_type in Release Debug; do
        candidate="${SCRIPT_DIR}/../build/${build_type}"
        if [ -d "${candidate}" ]; then
            BUILD_DIR="${candidate}"
            break
        fi
    done
    
    # Fallback to Debug if nothing found
    if [ -z "${BUILD_DIR}" ]; then
        BUILD_DIR="${SCRIPT_DIR}/../build/Debug"
    fi
fi

OUT_DIR="${SCRIPT_DIR}/../out"
JAVA_BUILD_DIR="${BUILD_DIR}/Core/ClientSMLSWIG/java"
JAVA_LIBRARY_DIR="${BUILD_DIR}/Core/ClientSMLSWIG"
if [ -f "${OUT_DIR}/java/sml.jar" ]; then
    JAVA_BUILD_DIR="${OUT_DIR}/java"
fi
if [ -f "${OUT_DIR}/libJava_sml_ClientInterface.so" ] || [ -f "${OUT_DIR}/libJava_sml_ClientInterface.jnilib" ] || [ -f "${OUT_DIR}/Java_sml_ClientInterface.dll" ]; then
    JAVA_LIBRARY_DIR="${OUT_DIR}"
fi

echo "Testing Java SWIG bindings..."
echo "Build directory: ${BUILD_DIR}"

# Check if required files exist
if [ ! -f "${JAVA_BUILD_DIR}/sml.jar" ]; then
    echo "Error:  sml.jar not found at ${JAVA_BUILD_DIR}/sml.jar"
    echo "Make sure Java SWIG bindings are built."
    exit 1
fi

# Check for native library
NATIVE_LIB=""
if [ -f "${JAVA_LIBRARY_DIR}/libJava_sml_ClientInterface.so" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/libJava_sml_ClientInterface.so"
elif [ -f "${JAVA_LIBRARY_DIR}/Java_sml_ClientInterface.so" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/Java_sml_ClientInterface.so"
elif [ -f "${JAVA_LIBRARY_DIR}/libJava_sml_ClientInterface.dylib" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/libJava_sml_ClientInterface.dylib"
elif [ -f "${JAVA_LIBRARY_DIR}/Java_sml_ClientInterface.dll" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/Java_sml_ClientInterface.dll"
elif [ -f "${JAVA_LIBRARY_DIR}/libsml.so" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/libsml.so"
elif [ -f "${JAVA_LIBRARY_DIR}/libsml.dylib" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/libsml.dylib"
elif [ -f "${JAVA_LIBRARY_DIR}/sml.dll" ]; then
    NATIVE_LIB="${JAVA_LIBRARY_DIR}/sml.dll"
fi

if [ -z "${NATIVE_LIB}" ]; then
    echo "[ERROR] Native library not found in ${JAVA_LIBRARY_DIR}/"
    exit 1
fi

echo "[OK] Found JAR file: ${JAVA_BUILD_DIR}/sml.jar"
echo "[OK] Found native library: ${NATIVE_LIB}"

# Create temporary directory for compilation
TEMP_DIR=$(mktemp -d)
trap "rm -rf ${TEMP_DIR}" EXIT

# Compile the test
echo "Compiling Java test..."
javac -cp "${JAVA_BUILD_DIR}/sml.jar" -d "${TEMP_DIR}" "${SCRIPT_DIR}/SimpleJavaSMLTest.java"

if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to compile Java test"
    exit 1
fi

echo "[OK] Java test compiled successfully"

# Run the test
echo "Running Java test..."
java -cp "${JAVA_BUILD_DIR}/sml.jar:${TEMP_DIR}" \
     -Djava.library.path="${JAVA_LIBRARY_DIR}" \
     SimpleJavaSMLTest

if [ $? -eq 0 ]; then
    echo "SUCCESS: Java SWIG bindings test passed!"
    exit 0
else
    echo "[ERROR] Java SWIG bindings test failed!"
    exit 1
fi