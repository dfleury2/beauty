from conans import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

import os

# -----------------------------------------------------------------------------
class BeautyConan(ConanFile):
    name            = "beauty"
    description     = "HTTP Server above Boost.Beast"
    version         = "1.0.3"
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

    def requirements(self):
        self.requires("boost/1.81.0@")
        if self.options.openssl:
            self.requires("openssl/1.1.1t@")

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

        # layout info
        self.cpp.build.includedirs = ["src"]
        self.cpp.build.libdirs = ["lib"]
        self.cpp.build.libs = ["beauty"]

        self.cpp.source.includedirs = ["include"]


        # build folder detection for editable mode
        conan_folders_build = os.getenv("CONAN_FOLDERS_BUILD", "build")
        if "CONAN_FOLDERS_BUILD" not in os.environ:
            build_type = str(self.settings.build_type).lower()
            if os.path.isdir(os.path.join(self.recipe_folder, f"cmake-build-{build_type}")):
                conan_folders_build = f"cmake-build-{build_type}"

        self.folders.build = conan_folders_build
        print(f"-- Conan folders build {self.folders.build}")
        self.folders.generators = self.folders.build
