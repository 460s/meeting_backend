from conans import ConanFile, CMake


class GetUserConan(ConanFile):
    name = "get_user"
    version = "0.0.1"
    description = "Library for students"
    license = "MIT"
    url = "git@github.com:460s/meeting_backend.git"
    settings = "os", "cppstd", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=True"
    generators = "cmake"
    exports_sources = "src/*", "include/*", "CMakeLists.txt"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder=".")
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include", src="include")
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so*", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["get_user"]
