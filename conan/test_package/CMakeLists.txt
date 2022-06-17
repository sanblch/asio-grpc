cmake_minimum_required(VERSION 3.1)
project(test_package)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(asio-grpc REQUIRED CONFIG)

add_library(asio-grpc-example-helper INTERFACE)

target_link_libraries(asio-grpc-example-helper INTERFACE gRPC::gRPC)

target_include_directories(asio-grpc-example-helper INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/helper")

# example protos
asio_grpc_protobuf_generate(
  GENERATE_GRPC GENERATE_MOCK_CODE
  OUT_VAR "ASIO_GRPC_EXAMPLE_PROTO_SOURCES"
  OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
  IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto"
  PROTOS "${CMAKE_CURRENT_LIST_DIR}/proto/helloworld/helloworld.proto")

function(asio_grpc_add_example_proto_target _asio_grpc_name)
  add_library(asio-grpc-${_asio_grpc_name} OBJECT)

  target_sources(asio-grpc-${_asio_grpc_name} PRIVATE ${ASIO_GRPC_EXAMPLE_PROTO_SOURCES})

  target_link_libraries(asio-grpc-${_asio_grpc_name} PUBLIC protobuf::libprotobuf gRPC::gRPC)

  target_include_directories(asio-grpc-${_asio_grpc_name}
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>")
endfunction()

function(asio_grpc_add_example _asio_grpc_name _asio_grpc_cxx_standard)
  add_executable(asio-grpc-example-${_asio_grpc_name})

  target_sources(asio-grpc-example-${_asio_grpc_name} PRIVATE ${_asio_grpc_name}.cpp)

  target_link_libraries(asio-grpc-example-${_asio_grpc_name} PRIVATE asio-grpc-example-helper)

  target_link_libraries(asio-grpc-example-${_asio_grpc_name} PRIVATE asio-grpc-example-protos-cpp20)
endfunction()

asio_grpc_add_example_proto_target(example-protos-cpp20)
target_compile_options(asio-grpc-example-protos-cpp20 PUBLIC "-fcoroutines")
target_compile_features(asio-grpc-example-protos-cpp20 PUBLIC cxx_std_20)

asio_grpc_add_example(hello-world-client "20")
target_link_libraries(asio-grpc-example-hello-world-client PRIVATE asio-grpc::asio-grpc gRPC::gRPC)
