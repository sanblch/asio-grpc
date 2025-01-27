# asio-grpc

[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Tradias_asio-grpc&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=Tradias_asio-grpc) [![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Tradias_asio-grpc&metric=coverage)](https://sonarcloud.io/dashboard?id=Tradias_asio-grpc) [![vcpkg](https://repology.org/badge/version-for-repo/vcpkg/asio-grpc.svg?header=vcpkg)](https://repology.org/project/asio-grpc/versions) [![hunter](https://img.shields.io/badge/hunter-asio_grpc-green.svg)](https://hunter.readthedocs.io/en/latest/packages/pkg/asio-grpc.html)

An [Executor, Networking TS](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Executor1.html#boost_asio.reference.Executor1.standard_executors) and [std::execution](https://brycelelbach.github.io/wg21_p2300_std_execution/std_execution.html) interface to [grpc::CompletionQueue](https://grpc.github.io/grpc/cpp/classgrpc_1_1_completion_queue.html) for writing asynchronous gRPC clients and servers using C++20 coroutines, Boost.Coroutines, Asio's stackless coroutines, callbacks, sender/receiver and more.

# Features

* Asio [ExecutionContext](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/ExecutionContext.html) compatible wrapper around [grpc::CompletionQueue](https://grpc.github.io/grpc/cpp/classgrpc_1_1_completion_queue.html)
* [Executor and Networking TS](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Executor1.html#boost_asio.reference.Executor1.standard_executors) requirements fulfilling associated executor
* Support for all RPC types: unary, client-streaming, server-streaming and bidirectional-streaming with any mix of Asio [CompletionToken](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/asynchronous_operations.html#boost_asio.reference.asynchronous_operations.completion_tokens_and_handlers) as well as  [TypedSender](https://github.com/facebookexperimental/libunifex/blob/main/doc/concepts.md#typedsender-concept), including allocator customization
* Support for asynchronously waiting for [grpc::Alarm](https://grpc.github.io/grpc/cpp/classgrpc_1_1_alarm.html)s including cancellation through [cancellation_slot](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/cancellation_slot.html)s and [StopToken](https://github.com/facebookexperimental/libunifex/blob/main/doc/concepts.md#stoptoken-concept)s
* Initial support for `std::execution` concepts through [libunifex](https://github.com/facebookexperimental/libunifex) and Asio: [schedule](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/execution__schedule.html), [connect](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/execution__connect.html), [submit](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/execution__submit.html), [scheduler](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Scheduler.html), [typed_sender](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Sender.html#boost_asio.reference.Sender.typed_sender) and more
* Support for generic gRPC clients and servers (aka. proxies)
* Experimental support for Rust/Golang [select](https://go.dev/ref/spec#Select_statements)-style programming with the help of [cancellation safety](https://tradias.github.io/asio-grpc/classagrpc_1_1_basic_grpc_stream.html)
* No-Boost version with [standalone Asio](https://github.com/chriskohlhoff/asio)
* No-Asio version with [libunifex](https://github.com/facebookexperimental/libunifex)
* CMake function to generate gRPC source files: [asio_grpc_protobuf_generate](/cmake/AsioGrpcProtobufGenerator.cmake)

# Example

* Server side 'hello world':

<!-- snippet: server-side-helloworld -->
<a id='snippet-server-side-helloworld'></a>
```cpp
std::unique_ptr<grpc::Server> server;

grpc::ServerBuilder builder;
agrpc::GrpcContext grpc_context{builder.AddCompletionQueue()};
builder.AddListeningPort(host, grpc::InsecureServerCredentials());
helloworld::Greeter::AsyncService service;
builder.RegisterService(&service);
server = builder.BuildAndStart();

boost::asio::co_spawn(
    grpc_context,
    [&]() -> boost::asio::awaitable<void>
    {
        grpc::ServerContext server_context;
        helloworld::HelloRequest request;
        grpc::ServerAsyncResponseWriter<helloworld::HelloReply> writer{&server_context};
        co_await agrpc::request(&helloworld::Greeter::AsyncService::RequestSayHello, service, server_context,
                                request, writer);
        helloworld::HelloReply response;
        response.set_message("Hello " + request.name());
        co_await agrpc::finish(writer, response, grpc::Status::OK);
    },
    boost::asio::detached);

grpc_context.run();
```
<sup><a href='/example/hello-world-server.cpp#L32-L58' title='Snippet source file'>snippet source</a> | <a href='#snippet-server-side-helloworld' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

More examples for things like streaming RPCs, double-buffered file transfer with io_uring, libunifex-based coroutines, sharing a thread with an io_context and generic clients/servers can be found in the [example](/example) directory. Even more examples can be found in another [repository](https://github.com/Tradias/example-vcpkg-grpc#branches).

# Requirements

Tested by CI:

 * CMake 3.16.3 (min. 3.14)
 * gRPC 1.44.0, 1.16.1 (older versions might work as well)
 * [Boost](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio.html) 1.79.0 (min. 1.74.0)
 * [Standalone Asio](https://github.com/chriskohlhoff/asio) 1.17.0 (min. 1.17.0)
 * [libunifex](https://github.com/facebookexperimental/libunifex) 2022-02-09
 * MSVC 19.32 (Visual Studio 17 2022)
 * GCC 8.4.0, 9.3.0, 10.3.0, 11.1.0
 * Clang 10.0.0, 11.0.0, 12.0.0
 * AppleClang 13.0.0.13000029
 * C++17 and C++20

For MSVC compilers and asio-grpc before v1.6.0 the following compile definitions need to be set:

```
BOOST_ASIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT
BOOST_ASIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT
BOOST_ASIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT
BOOST_ASIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT
BOOST_ASIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT
BOOST_ASIO_HAS_DEDUCED_PREFER_MEMBER_TRAIT
```

When using [standalone Asio](https://github.com/chriskohlhoff/asio) then omit the `BOOST_` prefix.

# Usage

The library can be added to a CMake project using either `add_subdirectory` or `find_package`. Once set up, include the individual headers from the agrpc/ directory or the combined header:

```cpp
#include <agrpc/asioGrpc.hpp>
```

<details><summary><b>As a subdirectory</b></summary>
<p>

Clone the repository into a subdirectory of your CMake project. Then add it and link it to your target.

Using [Boost.Asio](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio.html):

```cmake
find_package(gRPC)
find_package(Boost)
add_subdirectory(/path/to/repository/root)
target_link_libraries(your_app PUBLIC gRPC::grpc++ asio-grpc::asio-grpc Boost::headers)
```

Or using [standalone Asio](https://github.com/chriskohlhoff/asio):

```cmake
find_package(gRPC)
find_package(asio)
add_subdirectory(/path/to/repository/root)
target_link_libraries(your_app PUBLIC gRPC::grpc++ asio-grpc::asio-grpc-standalone-asio asio::asio)
```

Or using [libunifex](https://github.com/facebookexperimental/libunifex):

```cmake
find_package(gRPC)
find_package(unifex)
add_subdirectory(/path/to/repository/root)
target_link_libraries(your_app PUBLIC gRPC::grpc++ asio-grpc::asio-grpc-unifex unifex::unifex)
```

</p>
</details>

<details><summary><b>As a CMake package</b></summary>
<p>

Clone the repository and install it.

```shell
cmake -B build -DCMAKE_INSTALL_PREFIX=/desired/installation/directory .
cmake --build build --target install
```

Locate it and link it to your target.

Using [Boost.Asio](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio.html):

```cmake
# Make sure CMAKE_PREFIX_PATH contains /desired/installation/directory
find_package(asio-grpc)
target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc)
```

Or using [standalone Asio](https://github.com/chriskohlhoff/asio):

```cmake
# Make sure CMAKE_PREFIX_PATH contains /desired/installation/directory
find_package(asio-grpc)
target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc-standalone-asio)
```

Or using [libunifex](https://github.com/facebookexperimental/libunifex):

```cmake
# Make sure CMAKE_PREFIX_PATH contains /desired/installation/directory
find_package(asio-grpc)
target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc-unifex)
```

</p>
</details>

<details><summary><b>Using vcpkg</b></summary>
<p>

Add [asio-grpc](https://github.com/microsoft/vcpkg/blob/master/ports/asio-grpc/vcpkg.json) to the dependencies inside your `vcpkg.json`: 

```
{
    "name": "your_app",
    "version": "0.1.0",
    "dependencies": [
        "asio-grpc",
        // To use the Boost.Asio backend add
        // "boost-asio",
        // To use the standalone Asio backend add
        // "asio",
        // To use the libunifex backend add
        // "libunifex"
    ]
}
```

Locate asio-grpc and link it to your target in your `CMakeLists.txt`:

```cmake
find_package(asio-grpc)
# Using the Boost.Asio backend
target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc)
# Or use the standalone Asio backend
#target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc-standalone-asio)
# Or use the libunifex backend
#target_link_libraries(your_app PUBLIC asio-grpc::asio-grpc-unifex)
```

### Available features

`boost-container` - Use Boost.Container instead of `<memory_resource>`

See [selecting-library-features](https://vcpkg.io/en/docs/users/selecting-library-features.html) to learn how to select features with vcpkg.

</p>
</details>

<details><summary><b>Using Hunter</b></summary>
<p>

See asio-grpc's documentation on the Hunter website: [https://hunter.readthedocs.io/en/latest/packages/pkg/asio-grpc.html](https://hunter.readthedocs.io/en/latest/packages/pkg/asio-grpc.html).

</p>
</details>

## CMake Options

`ASIO_GRPC_USE_BOOST_CONTAINER` - Use Boost.Container instead of `<memory_resource>`.

`ASIO_GRPC_DISABLE_AUTOLINK` - Set before using `find_package(asio-grpc)` to prevent `asio-grpcConfig.cmake` from finding and setting up interface link libraries.

# Performance

asio-grpc is part of [grpc_bench](https://github.com/Tradias/grpc_bench). Head over there to compare its performance against other libraries and languages.

Results from the helloworld unary RPC   
Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz, Linux, GCC 12.1.0, Boost 1.79.0, gRPC 1.46.3, asio-grpc v1.7.0, jemalloc 5.2.1   
Request scenario: string_100B

<details><summary><b>Results</b></summary>
<p>

### 1 CPU server

| name                        |   req/s |   avg. latency |        90 % in |        95 % in |        99 % in | avg. cpu |   avg. memory |
|-----------------------------|--------:|---------------:|---------------:|---------------:|---------------:|---------:|--------------:|
| go_grpc                     |   48622 |       19.85 ms |       30.16 ms |       33.44 ms |       40.17 ms |  101.95% |     24.58 MiB |
| rust_tonic_mt               |   41007 |       24.21 ms |       10.57 ms |       11.63 ms |      637.63 ms |  100.66% |     13.74 MiB |
| rust_thruster_mt            |   40807 |       24.35 ms |       10.70 ms |       12.33 ms |      630.52 ms |  103.65% |     11.51 MiB |
| cpp_asio_grpc_unifex        |   37288 |       26.69 ms |       28.34 ms |       28.79 ms |       30.88 ms |  103.08% |     29.52 MiB |
| cpp_grpc_mt                 |   37078 |       26.84 ms |       28.55 ms |       29.02 ms |       30.14 ms |  103.15% |     29.05 MiB |
| cpp_asio_grpc_callback      |   36801 |       27.05 ms |       28.77 ms |       29.22 ms |       30.70 ms |  102.32% |      29.2 MiB |
| rust_grpcio                 |   35994 |       27.67 ms |       29.54 ms |       30.23 ms |       31.14 ms |  102.77% |     17.19 MiB |
| cpp_asio_grpc_coroutine     |   32393 |       30.74 ms |       32.91 ms |       33.53 ms |       35.01 ms |  102.54% |     27.11 MiB |
| cpp_asio_grpc_io_context_coro |   30757 |       32.38 ms |       34.51 ms |       35.06 ms |       36.33 ms |   77.94% |      26.5 MiB |
| cpp_grpc_callback           |   10800 |       84.36 ms |      108.19 ms |      155.96 ms |      171.98 ms |  101.62% |     65.79 MiB |

### 2 CPU server

| name                        |   req/s |   avg. latency |        90 % in |        95 % in |        99 % in | avg. cpu |   avg. memory |
|-----------------------------|--------:|---------------:|---------------:|---------------:|---------------:|---------:|--------------:|
| cpp_asio_grpc_unifex        |   85002 |        9.84 ms |       15.42 ms |       18.54 ms |       27.56 ms |  195.27% |     85.67 MiB |
| cpp_asio_grpc_callback      |   84842 |        9.90 ms |       15.44 ms |       18.52 ms |       27.20 ms |  197.58% |     80.47 MiB |
| cpp_grpc_mt                 |   84513 |        9.89 ms |       15.41 ms |       18.51 ms |       27.40 ms |  198.88% |     79.34 MiB |
| cpp_asio_grpc_coroutine     |   80263 |       10.65 ms |       16.77 ms |       19.82 ms |       27.94 ms |  211.12% |     80.35 MiB |
| cpp_asio_grpc_io_context_coro |   76454 |       11.35 ms |       18.42 ms |       21.51 ms |       29.58 ms |  158.83% |     75.56 MiB |
| cpp_grpc_callback           |   74806 |       10.87 ms |       19.12 ms |       23.23 ms |       31.87 ms |  209.88% |    131.14 MiB |
| go_grpc                     |   67022 |       13.04 ms |       20.26 ms |       23.44 ms |       30.73 ms |  197.34% |     24.42 MiB |
| rust_thruster_mt            |   61205 |       15.14 ms |       43.51 ms |       74.85 ms |       96.47 ms |  201.97% |     13.13 MiB |
| rust_grpcio                 |   60897 |       15.36 ms |       22.58 ms |       25.41 ms |       30.79 ms |  212.83% |     31.49 MiB |
| rust_tonic_mt               |   59668 |       15.74 ms |       42.14 ms |       62.74 ms |       97.83 ms |  202.56% |      15.2 MiB |

</p>
</details>

# Documentation

[**API reference**](https://tradias.github.io/asio-grpc/)

The main workhorses of this library are the `agrpc::GrpcContext` and its `executor_type` - `agrpc::GrpcExecutor`. 

The `agrpc::GrpcContext` implements [asio::execution_context](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/execution_context.html) and can be used as an argument to Asio functions that expect an `ExecutionContext` like [asio::spawn](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/spawn/overload7.html).

Likewise, the `agrpc::GrpcExecutor` satisfies the [Executor and Networking TS](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Executor1.html#boost_asio.reference.Executor1.standard_executors) and [Scheduler](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Scheduler.html) requirements and can therefore be used in places where Asio/libunifex expects an `Executor` or `Scheduler`.

The API for RPCs is modeled closely after the asynchronous, tag-based API of gRPC. As an example, the equivalent for `grpc::ClientAsyncReader<helloworld::HelloReply>.Read(helloworld::HelloReply*, void*)` would be `agrpc::read(grpc::ClientAsyncReader<helloworld::HelloReply>&, helloworld::HelloReply&, CompletionToken)`.

Instead of the `void*` tag in the gRPC API the functions in this library expect a [CompletionToken](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/asynchronous_operations.html#boost_asio.reference.asynchronous_operations.completion_tokens_and_handlers). Asio comes with several CompletionTokens already: [C++20 coroutine](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/use_awaitable.html), [stackless coroutine](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/coroutine.html), [callback](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/executor_binder.html) and [Boost.Coroutine](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/basic_yield_context.html). There is also a special token created by `agrpc::use_sender(scheduler)` that causes RPC functions to return a [TypedSender](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/reference/Sender.html#boost_asio.reference.Sender.typed_sender).

If you are interested in learning more about the implementation details of this library then check out [this blog article](https://medium.com/3yourmind/c-20-coroutines-for-asynchronous-grpc-services-5b3dab1d1d61).

<details><summary><b>Getting started</b></summary>
<p>

## Getting started

Start by creating a `agrpc::GrpcContext`.

For servers and clients:

<!-- snippet: create-grpc_context-server-side -->
<a id='snippet-create-grpc_context-server-side'></a>
```cpp
grpc::ServerBuilder builder;
agrpc::GrpcContext grpc_context{builder.AddCompletionQueue()};
```
<sup><a href='/example/snippets/server.cpp#L324-L327' title='Snippet source file'>snippet source</a> | <a href='#snippet-create-grpc_context-server-side' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

For clients only:

<!-- snippet: create-grpc_context-client-side -->
<a id='snippet-create-grpc_context-client-side'></a>
```cpp
agrpc::GrpcContext grpc_context{std::make_unique<grpc::CompletionQueue>()};
```
<sup><a href='/example/snippets/client.cpp#L348-L350' title='Snippet source file'>snippet source</a> | <a href='#snippet-create-grpc_context-client-side' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

Add some work to the `grpc_context` and run it. Make sure to shutdown the `server` before destructing the `grpc_context`. Also destruct the `grpc_context` before destructing the `server`. A `grpc_context` can only be run on one thread at a time.

<!-- snippet: run-grpc_context-server-side -->
<a id='snippet-run-grpc_context-server-side'></a>
```cpp
grpc_context.run();
server->Shutdown();
}  // grpc_context is destructed here before the server
```
<sup><a href='/example/snippets/server.cpp#L342-L346' title='Snippet source file'>snippet source</a> | <a href='#snippet-run-grpc_context-server-side' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

It might also be helpful to create a work guard before running the `agrpc::GrpcContext` to prevent `grpc_context.run()` from returning early.

<!-- snippet: make-work-guard -->
<a id='snippet-make-work-guard'></a>
```cpp
std::optional guard{asio::require(grpc_context.get_executor(), asio::execution::outstanding_work_t::tracked)};
```
<sup><a href='/example/snippets/client.cpp#L352-L354' title='Snippet source file'>snippet source</a> | <a href='#snippet-make-work-guard' title='Start of snippet'>anchor</a></sup>
<!-- endSnippet -->

## Where to go from here?

Check out the [examples](/example) and the [API documentation](https://tradias.github.io/asio-grpc/).

</p>
</details>


# What users are saying

> Asio-grpc abstracts away the implementation details of asynchronous grpc handling: crafting working code is easier, faster, less prone to errors and considerably more fun. At 3YOURMIND we reliably use asio-grpc in production since its very first release, allowing our developers to effortlessly implement low-latency/high-throughput asynchronous data transfer in time critical applications.

[@3YOURMIND](https://github.com/3YOURMIND)

> Our project is a real-time distributed motion capture system that uses your framework to stream data back and forward between multiple machines. Previously I have tried to build a bidirectional streaming framework from scratch using only gRPC. However, it's not maintainable and error-prone due to a large amount of service and streaming code. As a developer whose experienced both raw grpc and asio-grpc, I can tell that your framework is a real a game-changer for writing grpc code in C++. It has made my life much easier. I really appreciate the effort you have put into this project and your superior skills in designing c++ template code.

[@khanhha](https://github.com/khanhha)
