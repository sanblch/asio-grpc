# Copyright 2022 Dennis Hezel
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# asio-grpc header check
if(ASIO_GRPC_ENABLE_CHECK_HEADER_SYNTAX_TARGET)
    add_library(asio-grpc-check-header-syntax OBJECT)

    function(asio_grpc_add_public_headers_as_sources _asio_grpc_target)
        get_target_property(_asio_grpc_public_header asio-grpc-sources INTERFACE_SOURCES)

        mark_as_advanced(_asio_grpc_public_header)

        list(FILTER _asio_grpc_public_header EXCLUDE REGEX ".*/agrpc/detail/.*")

        target_sources(${_asio_grpc_target} PRIVATE ${_asio_grpc_public_header})

        set_source_files_properties(${_asio_grpc_public_header} PROPERTIES LANGUAGE CXX)
    endfunction()

    asio_grpc_add_public_headers_as_sources(asio-grpc-check-header-syntax)

    target_compile_options(
        asio-grpc-check-header-syntax
        PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/Zs;/WX,-Werror;-fsyntax-only;-Wno-unused-const-variable>)

    target_link_libraries(asio-grpc-check-header-syntax PRIVATE asio-grpc-sources asio-grpc-common-compile-options
                                                                asio-grpc)

    set_target_properties(asio-grpc-check-header-syntax PROPERTIES EXCLUDE_FROM_ALL on)
endif()

# utils
add_subdirectory(utils)

# tests
function(asio_grpc_add_test _asio_grpc_name _asio_grpc_type _asio_grpc_cxx_standard)
    add_executable(${_asio_grpc_name})

    target_sources(${_asio_grpc_name} PRIVATE ${ARGN})

    if(${_asio_grpc_cxx_standard} STREQUAL "20")
        set(_asio_grpc_cxx_standard "-cpp20")
    else()
        unset(_asio_grpc_cxx_standard)
    endif()

    target_link_libraries(${_asio_grpc_name} PRIVATE "asio-grpc-test-main${_asio_grpc_cxx_standard}"
                                                     "asio-grpc-test-protobuf${_asio_grpc_cxx_standard}")
    if(${_asio_grpc_type} STREQUAL "BOOST_ASIO")
        target_link_libraries(${_asio_grpc_name} PRIVATE "asio-grpc-test-util-boost-asio${_asio_grpc_cxx_standard}")

        target_precompile_headers(${_asio_grpc_name} REUSE_FROM
                                  "asio-grpc-test-util-boost-asio${_asio_grpc_cxx_standard}")
    elseif(${_asio_grpc_type} STREQUAL "STANDALONE_ASIO")
        target_link_libraries(${_asio_grpc_name}
                              PRIVATE "asio-grpc-test-util-standalone-asio${_asio_grpc_cxx_standard}")

        target_precompile_headers(${_asio_grpc_name} REUSE_FROM
                                  "asio-grpc-test-util-standalone-asio${_asio_grpc_cxx_standard}")
    elseif(${_asio_grpc_type} STREQUAL "UNIFEX")
        target_link_libraries(${_asio_grpc_name} PRIVATE "asio-grpc-test-util-unifex${_asio_grpc_cxx_standard}")

        target_precompile_headers(${_asio_grpc_name} REUSE_FROM "asio-grpc-test-util-unifex${_asio_grpc_cxx_standard}")
    endif()

    if(ASIO_GRPC_DISCOVER_TESTS)
        doctest_discover_tests(${_asio_grpc_name} ADD_LABELS 0)
    endif()
endfunction()

set(ASIO_GRPC_CPP17_TEST_SOURCE_FILES
    "testAsioGrpc17.cpp"
    "testRepeatedlyRequest17.cpp"
    "testBindAllocator17.cpp"
    "testGrpcContext17.cpp"
    "testRun17.cpp"
    "testGenericRpc17.cpp"
    "testGrpcStream17.cpp"
    "testTest17.cpp")
set(ASIO_GRPC_CPP20_TEST_SOURCE_FILES "testAsioGrpc20.cpp" "testRepeatedlyRequest20.cpp" "testBindAllocator20.cpp"
                                      "testGrpcContext20.cpp" "testGrpcStream20.cpp")

asio_grpc_add_test(asio-grpc-test-boost-cpp17 "BOOST_ASIO" "17" ${ASIO_GRPC_CPP17_TEST_SOURCE_FILES})
target_compile_definitions(asio-grpc-test-boost-cpp17 PRIVATE "ASIO_GRPC_TEST_CPP_VERSION=\"Boost.Asio C++17\"")
target_link_libraries(asio-grpc-test-boost-cpp17 PRIVATE Boost::coroutine Boost::thread)

asio_grpc_add_test(asio-grpc-test-cpp17 "STANDALONE_ASIO" "17" ${ASIO_GRPC_CPP17_TEST_SOURCE_FILES})
target_compile_definitions(asio-grpc-test-cpp17 PRIVATE "ASIO_GRPC_TEST_CPP_VERSION=\"Standalone Asio C++17\"")
target_link_libraries(asio-grpc-test-cpp17 PRIVATE Boost::coroutine Boost::thread)

if(ASIO_GRPC_ENABLE_CPP20_TESTS_AND_EXAMPLES)
    asio_grpc_add_test(asio-grpc-test-boost-cpp20 "BOOST_ASIO" "20" ${ASIO_GRPC_CPP17_TEST_SOURCE_FILES}
                       ${ASIO_GRPC_CPP20_TEST_SOURCE_FILES})
    target_compile_definitions(asio-grpc-test-boost-cpp20 PRIVATE "ASIO_GRPC_TEST_CPP_VERSION=\"Boost.Asio C++20\"")
    target_link_libraries(asio-grpc-test-boost-cpp20 PRIVATE Boost::coroutine Boost::thread)

    asio_grpc_add_test(asio-grpc-test-cpp20 "STANDALONE_ASIO" "20" ${ASIO_GRPC_CPP17_TEST_SOURCE_FILES}
                       ${ASIO_GRPC_CPP20_TEST_SOURCE_FILES})
    target_compile_definitions(asio-grpc-test-cpp20 PRIVATE "ASIO_GRPC_TEST_CPP_VERSION=\"Standalone Asio C++20\"")
    target_link_libraries(asio-grpc-test-cpp20 PRIVATE Boost::coroutine Boost::thread)

    asio_grpc_add_test(asio-grpc-test-unifex-cpp20 "UNIFEX" "20" "testUnifex20.cpp")
    target_compile_definitions(asio-grpc-test-unifex-cpp20 PRIVATE "ASIO_GRPC_TEST_CPP_VERSION=\"unifex C++20\"")
endif()

unset(ASIO_GRPC_CPP17_TEST_SOURCE_FILES)
unset(ASIO_GRPC_CPP20_TEST_SOURCE_FILES)

# tests for examples
if(ASIO_GRPC_BOOST_ASIO_HAS_CO_AWAIT AND ASIO_GRPC_ENABLE_CPP20_TESTS_AND_EXAMPLES)
    add_executable(asio-grpc-test-examples)

    target_sources(asio-grpc-test-examples PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/testExamples.cpp")

    target_link_libraries(asio-grpc-test-examples PRIVATE asio-grpc-test-main Boost::filesystem)

    add_dependencies(
        asio-grpc-test-examples
        asio-grpc-example-streaming-client
        asio-grpc-example-streaming-server
        asio-grpc-example-share-io-context-client
        asio-grpc-example-share-io-context-server
        asio-grpc-example-hello-world-client
        asio-grpc-example-hello-world-server
        asio-grpc-example-unifex-client
        asio-grpc-example-unifex-server
        asio-grpc-example-generic-client
        asio-grpc-example-generic-server
        asio-grpc-example-multi-threaded-client
        asio-grpc-example-multi-threaded-server)

    set(ASIO_GRPC_TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/temp")
    make_directory("${ASIO_GRPC_TEMP_DIR}")

    target_compile_definitions(
        asio-grpc-test-examples
        PRIVATE "ASIO_GRPC_EXAMPLE_TEMP_DIR=\"${ASIO_GRPC_TEMP_DIR}\""
                "ASIO_GRPC_EXAMPLE_STREAMING_CLIENT=\"$<TARGET_FILE:asio-grpc-example-streaming-client>\""
                "ASIO_GRPC_EXAMPLE_STREAMING_SERVER=\"$<TARGET_FILE:asio-grpc-example-streaming-server>\""
                "ASIO_GRPC_EXAMPLE_SHARE_IO_CONTEXT_CLIENT=\"$<TARGET_FILE:asio-grpc-example-share-io-context-client>\""
                "ASIO_GRPC_EXAMPLE_SHARE_IO_CONTEXT_SERVER=\"$<TARGET_FILE:asio-grpc-example-share-io-context-server>\""
                "ASIO_GRPC_EXAMPLE_HELLO_WORLD_CLIENT=\"$<TARGET_FILE:asio-grpc-example-hello-world-client>\""
                "ASIO_GRPC_EXAMPLE_HELLO_WORLD_SERVER=\"$<TARGET_FILE:asio-grpc-example-hello-world-server>\""
                "ASIO_GRPC_EXAMPLE_UNIFEX_CLIENT=\"$<TARGET_FILE:asio-grpc-example-unifex-client>\""
                "ASIO_GRPC_EXAMPLE_UNIFEX_SERVER=\"$<TARGET_FILE:asio-grpc-example-unifex-server>\""
                "ASIO_GRPC_EXAMPLE_GENERIC_CLIENT=\"$<TARGET_FILE:asio-grpc-example-generic-client>\""
                "ASIO_GRPC_EXAMPLE_GENERIC_SERVER=\"$<TARGET_FILE:asio-grpc-example-generic-server>\""
                "ASIO_GRPC_EXAMPLE_MULTI_THREADED_CLIENT=\"$<TARGET_FILE:asio-grpc-example-multi-threaded-client>\""
                "ASIO_GRPC_EXAMPLE_MULTI_THREADED_SERVER=\"$<TARGET_FILE:asio-grpc-example-multi-threaded-server>\"")

    if(ASIO_GRPC_ENABLE_IO_URING_EXAMPLES OR ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        add_dependencies(asio-grpc-test-examples asio-grpc-example-file-transfer-client
                         asio-grpc-example-file-transfer-server)

        target_compile_definitions(
            asio-grpc-test-examples
            PRIVATE "ASIO_GRPC_EXAMPLE_FILE_TRANSFER_CLIENT=\"$<TARGET_FILE:asio-grpc-example-file-transfer-client>\""
                    "ASIO_GRPC_EXAMPLE_FILE_TRANSFER_SERVER=\"$<TARGET_FILE:asio-grpc-example-file-transfer-server>\"")
    endif()

    if(ASIO_GRPC_DISCOVER_TESTS)
        doctest_discover_tests(asio-grpc-test-examples ADD_LABELS 0)
    endif()
endif()

# cmake install test
if(ASIO_GRPC_ENABLE_CMAKE_INSTALL_TEST)
    set(ASIO_GRPC_CMAKE_INSTALL_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    configure_file("${ASIO_GRPC_CMAKE_INSTALL_TEST_SOURCE_DIR}/CMakeLists.txt.in"
                   "${ASIO_GRPC_CMAKE_INSTALL_TEST_SOURCE_DIR}/CMakeLists.txt" @ONLY)

    if("${CMAKE_GENERATOR}" STREQUAL "Ninja Multi-Config")
        if(WIN32)
            set(ASIO_GRPC_CMAKE_INSTALL_TEST_GENERATOR "NMake Makefiles")
        else()
            set(ASIO_GRPC_CMAKE_INSTALL_TEST_GENERATOR "Unix Makefiles")
        endif()
    else()
        set(ASIO_GRPC_CMAKE_INSTALL_TEST_GENERATOR "${CMAKE_GENERATOR}")
    endif()

    add_test(
        NAME asio-grpc-cmake-test
        COMMAND
            "${ASIO_GRPC_CMAKE_INSTALL_TEST_CTEST_COMMAND}" "--build-and-test"
            "${ASIO_GRPC_CMAKE_INSTALL_TEST_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/cmake-test" "--build-generator"
            "${ASIO_GRPC_CMAKE_INSTALL_TEST_GENERATOR}" "--build-project" "asio-grpc-cmake-test-superbuild"
            "--test-command" "${CMAKE_CURRENT_BINARY_DIR}/cmake-test/test-install/main${CMAKE_EXECUTABLE_SUFFIX}")
endif()
