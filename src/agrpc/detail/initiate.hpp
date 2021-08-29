// Copyright 2021 Dennis Hezel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AGRPC_DETAIL_INITIATE_HPP
#define AGRPC_DETAIL_INITIATE_HPP

#include "agrpc/detail/asioForward.hpp"
#include "agrpc/detail/grpcExecutorOperation.hpp"
#include "agrpc/grpcContext.hpp"

namespace agrpc::detail
{
template <class Function>
struct GrpcInitiator
{
    using executor_type = asio::associated_executor_t<Function>;
    using allocator_type = asio::associated_allocator_t<Function>;

    struct OnWork
    {
        agrpc::GrpcContext& grpc_context;
        Function& function;

        template <class Handler, class Allocator>
        void operator()(detail::GrpcExecutorOperation<Handler, Allocator>* work)
        {
            function(grpc_context, work);
        }
    };

    Function function;

    template <class CompletionHandler>
    void operator()(CompletionHandler completion_handler)
    {
        const auto [executor, allocator] = detail::get_associated_executor_and_allocator(completion_handler);
        auto& grpc_context = static_cast<agrpc::GrpcContext&>(asio::query(executor, asio::execution::context));
        detail::create_work<true>(grpc_context, std::move(completion_handler), OnWork{grpc_context, function},
                                  allocator);
    }

    executor_type get_executor() const noexcept { return asio::get_associated_executor(function); }

    allocator_type get_allocator() const noexcept { return asio::get_associated_allocator(function); }
};

template <class Function>
GrpcInitiator(Function&&) -> GrpcInitiator<Function>;
}  // namespace agrpc::detail

#endif  // AGRPC_DETAIL_INITIATE_HPP
