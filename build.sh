BUILD_TYPE="Release" # Possible values are ['Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']

rm -rf build/ install/
pip install conan
conan profile detect --force
conan install . --build=missing --settings=build_type=$BUILD_TYPE
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$BUILD_TYPE/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build build --config $BUILD_TYPE --parallel 8
cmake --install build --config $BUILD_TYPE
