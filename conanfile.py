from conans import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

# -----------------------------------------------------------------------------
class BeautyConan(ConanFile):
    name            = "beauty"
    description     = "HTTP Server above Boost.Beast"
    version         = "0.1-rc"
    url             = "https://github.com/dfleury2/beauty"
    license         = "To be defined"
    settings        = "os", "compiler", "build_type", "arch"
    options         = {"shared": [True, False]}
    default_options = "shared=False"

    requires        = ("boost/1.77.0@",
                       "openssl/1.1.1l@")

    exports_sources = "CMakeLists.txt", "include*", "src*", "examples*", "cmake/*"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="beauty")

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="beauty")
        cmake.install()

    def layout(self):
        # package_info
        self.cpp.package.includedirs = ["include"]
        self.cpp.package.libs = ["beauty"]
