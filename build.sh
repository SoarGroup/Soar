pip install conan
conan profile detect --force
conan install . --build=missing
conan install . --build=missing -s build_type=Debug
cmake --workflow --preset Debug-test-workflow
cmake --install build/Debug
