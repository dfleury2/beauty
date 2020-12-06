from conans import ConanFile, CMake, tools

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
    generators      = "cmake_paths"

    requires        = ("boost/1.74.0@",
                       "openssl/1.1.1g@")

    exports_sources = "*", "!build"

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(source_folder=".")
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ["include/" + self.name]
        self.cpp_info.libs = ["beauty"]
