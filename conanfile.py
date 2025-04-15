# This file defines the cmake toolchain with the required dependencies to build
# Soar. The Python version is used instead of the pure text version since the
# user presets file must be changed.
# 
# Conan documentation: https://docs.conan.io/2/

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class soarRecipe(ConanFile):
    name = "soar"
    version = "9.6.3"
    package_type = "application"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    def layout(self):
        cmake_layout(self)
    
    def requirements(self):
        self.requires("sqlite3/3.40.0")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # The cmake user presets file must be changed from the default,
        # otherwise cyclic imports with toolchain file CMakePresets.json will
        # occur
        tc.user_presets_path = 'ConanPresets.json'
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
