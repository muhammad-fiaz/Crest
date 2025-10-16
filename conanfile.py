from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os


class CrestConan(ConanFile):
    name = "crest"
    version = "1.0.0"
    license = "MIT"
    author = "Muhammad Fiaz"
    url = "https://github.com/muhammad-fiaz/crest"
    description = "A modern, fast, and lightweight REST API framework for C/C++"
    topics = ("rest", "api", "framework", "web", "http")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "dashboard": [True, False],
        "examples": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
        "dashboard": True,
        "examples": False,
    }
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "examples/*", "cmake/*"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
        # Pure C library
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["CREST_BUILD_SHARED"] = self.options.shared
        tc.variables["CREST_BUILD_STATIC"] = not self.options.shared
        tc.variables["CREST_ENABLE_DASHBOARD"] = self.options.dashboard
        tc.variables["CREST_BUILD_EXAMPLES"] = self.options.examples
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        copy(
            self,
            "*.h",
            src=os.path.join(self.source_folder, "include"),
            dst=os.path.join(self.package_folder, "include"),
            keep_path=True,
        )

    def package_info(self):
        self.cpp_info.libs = ["crest"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["ws2_32"]
        elif self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs = ["pthread"]
