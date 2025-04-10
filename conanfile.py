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
        tc.user_presets_path = 'ConanPresets.json'
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
