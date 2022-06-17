from conan.tools.microsoft import msvc_runtime_flag
from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
import os
import functools

required_conan_version = ">=1.43.0"


class GrpcAsioConan(ConanFile):
    name = "asio-grpc"
    description = "Google's C++ test framework"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/sanblch/asio-grpc"
    license = "Apache-2.0"
    topics = (
        "cpp",
        "asynchronous",
        "grpc",
        "asio",
        "cpp17",
        "coroutine",
        "cpp20",
        "executors",
        "libunifex",
    )

    settings = "os", "arch", "compiler", "build_type"

    exports_sources = "CMakeLists.txt"
    generators = "cmake", "cmake_find_package", "cmake_find_package_multi"
    _cmake = None

    @property
    def _source_subfolder(self):
        return "source_subfolder"

    def requirements(self):
        self.requires("boost/1.79.0")
        self.requires("grpc/1.45.2")

    def source(self):
        tools.get(
            **self.conan_data["sources"][self.version],
            destination=self._source_subfolder,
            strip_root=True
        )

    @property
    def _cmake_install_base_path(self):
        return os.path.join("lib", "cmake", "asio-grpc")

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake
        self._cmake = CMake(self)
        self._cmake.configure()
        return self._cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package_id(self):
        self.info.header_only()

    def package(self):
        self.copy("LICENSE", dst="licenses", src=self._source_subfolder)
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        build_modules = [
            os.path.join(
                self._cmake_install_base_path, "AsioGrpcProtobufGenerator.cmake"
            )
        ]

        self.cpp_info.set_property("cmake_build_modules", build_modules)
        self.cpp_info.build_modules["cmake_find_package"] = build_modules
        self.cpp_info.build_modules["cmake_find_package_multi"] = build_modules
        self.cpp_info.defines = ["AGRPC_BOOST_ASIO"]
