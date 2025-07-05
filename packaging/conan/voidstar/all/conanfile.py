from conan import ConanFile
from conan.tools.files import get, copy


class voidstarRecipe(ConanFile):
    name = "voidstar"
    package_type = "header-library"

    license = "EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0"
    author = "OLEGSHA (kvadropups+voidstar@gmail.com)"
    url = "https://github.com/OLEGSHA/voidstar"
    description = "Converts any C++ lambda to function pointer."
    topics = ("cpp20", "libffi")

    no_copy_source=True

    # Automatically manage the package ID clearing of settings and options
    implements = ["auto_header_only"]

    def source(self):
        get(self, **self.conan_data["sources"][self.version],  strip_root=True)

    def requirements(self):
        self.requires("libffi/[>=3]")

    def package(self):
        copy(self, "include/*", self.source_folder, self.package_folder)
        copy(self, "LICENSE", self.source_folder, self.package_folder)

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []

