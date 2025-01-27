# test protobuf sources
asio_grpc_protobuf_generate(
    GENERATE_GRPC GENERATE_MOCK_CODE
    OUT_VAR "ASIO_GRPC_TEXT_PROTO_SOURCES"
    OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/proto"
    PROTOS "${CMAKE_CURRENT_SOURCE_DIR}/proto/test/v1/test.proto"
           "${CMAKE_CURRENT_SOURCE_DIR}/proto/test/msg/message.proto")

function(asio_grpc_add_test_proto_target _asio_grpc_name)
    add_library(${_asio_grpc_name} OBJECT)

    target_sources(${_asio_grpc_name} PRIVATE ${ASIO_GRPC_TEXT_PROTO_SOURCES})

    target_link_libraries(${_asio_grpc_name} PUBLIC protobuf::libprotobuf)

    target_include_directories(${_asio_grpc_name} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>")
endfunction()

asio_grpc_add_test_proto_target(asio-grpc-test-protobuf)
target_compile_features(asio-grpc-test-protobuf PUBLIC cxx_std_17)

asio_grpc_add_test_proto_target(asio-grpc-test-protobuf-cpp20)
target_compile_features(asio-grpc-test-protobuf-cpp20 PUBLIC cxx_std_20)

# test main
function(asio_grpc_add_test_main _asio_grpc_name)
    add_library(${_asio_grpc_name} OBJECT)

    target_sources(
        ${_asio_grpc_name}
        PRIVATE # cmake-format: sort
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/clientContext.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/clientContext.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/doctest.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/freePort.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/freePort.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/main.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/time.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/time.hpp")

    target_include_directories(${_asio_grpc_name} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                                         $<INSTALL_INTERFACE:include>)

    target_link_libraries(
        ${_asio_grpc_name}
        PUBLIC doctest::doctest GTest::gmock asio-grpc-common-compile-options asio-grpc-coverage-options
        PRIVATE Boost::headers Boost::filesystem)
endfunction()

asio_grpc_add_test_main(asio-grpc-test-main)
target_link_libraries(asio-grpc-test-main PUBLIC asio-grpc-test-protobuf)

asio_grpc_add_test_main(asio-grpc-test-main-cpp20)
target_link_libraries(asio-grpc-test-main-cpp20 PUBLIC asio-grpc-test-protobuf-cpp20 asio-grpc-cpp20-compile-options)

# test utils
if(ASIO_GRPC_USE_BOOST_CONTAINER)
    set(ASIO_GRPC_MEMORY_RESOURCE_UTILS_INCLUDE [[#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/global_resource.hpp>]])
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/utils/memoryResource.hpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generated/utils/memoryResource.hpp" @ONLY)
unset(ASIO_GRPC_MEMORY_RESOURCE_UTILS_INCLUDE)

function(asio_grpc_add_test_util _asio_grpc_name _asio_grpc_type _asio_grpc_cxx_standard)
    add_library(${_asio_grpc_name} OBJECT)

    target_sources(
        ${_asio_grpc_name}
        PRIVATE # cmake-format: sort
                "${CMAKE_CURRENT_BINARY_DIR}/generated/utils/memoryResource.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/asioSeparateCompilation.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/asioUtils.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcClientServerTest.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcClientServerTest.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcClientServerTestBase.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcClientServerTestBase.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcContextTest.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcContextTest.hpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcGenericClientServerTest.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/utils/grpcGenericClientServerTest.hpp")
    if(NOT ${_asio_grpc_type} STREQUAL "UNIFEX")
        target_sources(
            ${_asio_grpc_name}
            PRIVATE # cmake-format: sort
                    "${CMAKE_CURRENT_SOURCE_DIR}/utils/rpc.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/utils/rpc.hpp")
    endif()

    target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc-common-compile-options asio-grpc-coverage-options)

    if(${_asio_grpc_type} STREQUAL "BOOST_ASIO")
        target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc)

        target_compile_definitions(${_asio_grpc_name} PUBLIC BOOST_ASIO_SEPARATE_COMPILATION BOOST_ASIO_NO_TS_EXECUTORS)
    elseif(${_asio_grpc_type} STREQUAL "STANDALONE_ASIO")
        target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc-standalone-asio)

        target_compile_definitions(${_asio_grpc_name} PUBLIC ASIO_SEPARATE_COMPILATION ASIO_NO_TS_EXECUTORS)
    elseif(${_asio_grpc_type} STREQUAL "UNIFEX")
        target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc-unifex)
    endif()

    target_include_directories(
        ${_asio_grpc_name}
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
               "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>" $<INSTALL_INTERFACE:include>)

    target_precompile_headers(${_asio_grpc_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/utils/precompiledHeader.hpp")

    if(${_asio_grpc_cxx_standard} STREQUAL "20")
        target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc-test-main-cpp20 asio-grpc-test-protobuf-cpp20)
    else()
        target_link_libraries(${_asio_grpc_name} PUBLIC asio-grpc-test-main asio-grpc-test-protobuf)
    endif()
endfunction()

asio_grpc_add_test_util(asio-grpc-test-util-boost-asio "BOOST_ASIO" "17")
asio_grpc_add_test_util(asio-grpc-test-util-standalone-asio "STANDALONE_ASIO" "17")

if(ASIO_GRPC_ENABLE_CPP20_TESTS_AND_EXAMPLES)
    asio_grpc_add_test_util(asio-grpc-test-util-boost-asio-cpp20 "BOOST_ASIO" "20")
    asio_grpc_add_test_util(asio-grpc-test-util-standalone-asio-cpp20 "STANDALONE_ASIO" "20")
    asio_grpc_add_test_util(asio-grpc-test-util-unifex-cpp20 "UNIFEX" "20")
endif()
