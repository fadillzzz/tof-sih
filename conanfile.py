import os
from conan import ConanFile

class TOFInternal(ConanFile):
    settings = "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        pass

    def build_requirements(self):
        self.tool_requires("cmake/3.31.6")

    def layout(self):
        is_msvc = True if self.settings.get_safe("compiler") == "msvc" else False
        if is_msvc:
            self.folders.generators = os.path.join("build", "generators")
            self.folders.build = "build"
        else:
            self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
            self.folders.build = os.path.join("build", str(self.settings.build_type))
