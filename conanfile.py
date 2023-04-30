from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

# -----------------------------------------------------------------------------
class BeautyConan(ConanFile):
    name            = "beauty"
    description     = "HTTP Server above Boost.Beast"
    version         = "1.0.0"
    url             = "https://github.com/dfleury2/beauty"
    license         = "MIT"
    settings        = "os", "compiler", "build_type", "arch"
    options         = {
        "shared": [True, False],
        "openssl": [True, False]
    }
    default_options = {
        "shared": False,
        "openssl": True
    }

    exports_sources = "CMakeLists.txt", "include*", "src*", "examples*", "cmake/*"

    def build_requirements(self):
        self.requires("boost/1.78.0@")
        if self.options.openssl:
            self.requires("openssl/1.1.1o@")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CONAN_IN_LOCAL_CACHE"] = self.in_local_cache
        tc.variables["BEAUTY_ENABLE_OPENSSL"] = self.options.openssl
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
