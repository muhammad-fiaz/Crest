# pylint: disable=import-error,no-member
# mypy: disable-error-code=import
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, cmake_layout
from conan.tools.files import copy


class CrestConan(ConanFile):
    """Conan recipe for the Crest RESTful API framework."""

    name = "crest"
    version = "0.0.0"
    license = "MIT"
    author = "Muhammad Fiaz <contact@muhammadfiaz.com>"
    url = "https://github.com/muhammad-fiaz/crest"
    description = "Production-ready RESTful API framework for C and C++"
    topics = ("rest", "api", "http", "server", "web")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "src/*", "include/*", "xmake.lua"

    def config_options(self):
        """Configure options based on the target platform."""
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        """Set up the directory layout for the build."""
        cmake_layout(self)

    def generate(self):
        """Generate build files using CMake toolchain."""
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        """Build the project using xmake."""
        # Use xmake for building
        self.run("xmake config -m %s" % str(self.settings.build_type).lower())
        self.run("xmake build crest")

    def package(self):
        """Package the built artifacts."""
        copy(self, "*.h", src=self.source_folder, dst=self.package_folder, keep_path=True)
        copy(self, "*.hpp", src=self.source_folder, dst=self.package_folder, keep_path=True)
        copy(self, "*.a", src=self.build_folder, dst=self.package_folder, keep_path=False)
        copy(self, "*.lib", src=self.build_folder, dst=self.package_folder, keep_path=False)
        copy(self, "*.so", src=self.build_folder, dst=self.package_folder, keep_path=False)
        copy(self, "*.dylib", src=self.build_folder, dst=self.package_folder, keep_path=False)

    def package_info(self):
        """Provide information about the packaged library."""
        self.cpp_info.libs = ["crest"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["ws2_32", "mswsock"]
        else:
            self.cpp_info.system_libs = ["pthread"]
