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

#ifndef AGRPC_DETAIL_REPEATEDLYREQUEST_HPP
#define AGRPC_DETAIL_REPEATEDLYREQUEST_HPP

#include "agrpc/detail/asioForward.hpp"
#include "agrpc/detail/config.hpp"
#include "agrpc/initiate.hpp"
#include "agrpc/rpcs.hpp"

#include <tuple>

AGRPC_NAMESPACE_BEGIN()

namespace detail
{
struct RepeatedlyRequestContextAccess
{
    template <class ImplementationAllocator>
    static constexpr auto create(detail::AllocatedPointer<ImplementationAllocator>&& allocated_pointer) noexcept
    {
        return agrpc::RepeatedlyRequestContext{std::move(allocated_pointer)};
    }
};

struct RPCContextBase
{
    grpc::ServerContext context{};

    constexpr auto& server_context() noexcept { return context; }
};

template <class Request, class Responder>
struct MultiArgRPCContext : detail::RPCContextBase
{
    Responder responder_{&this->context};
    Request request_{};

    template <class Handler, class... Args>
    constexpr decltype(auto) operator()(Handler&& handler, Args&&... args)
    {
        return std::invoke(std::forward<Handler>(handler), this->context, this->request_, this->responder_,
                           std::forward<Args>(args)...);
    }

    constexpr auto args() noexcept { return std::forward_as_tuple(this->context, this->request_, this->responder_); }

    constexpr auto& request() noexcept { return this->request_; }

    constexpr auto& responder() noexcept { return this->responder_; }
};

template <class Responder>
struct SingleArgRPCContext : detail::RPCContextBase
{
    Responder responder_{&this->context};

    template <class Handler, class... Args>
    constexpr decltype(auto) operator()(Handler&& handler, Args&&... args)
    {
        return std::invoke(std::forward<Handler>(handler), this->context, this->responder_,
                           std::forward<Args>(args)...);
    }

    constexpr auto args() noexcept { return std::forward_as_tuple(this->context, this->responder_); }

    constexpr auto& responder() noexcept { return this->responder_; }
};

template <class RPC, class Service, class RPCHandlerAllocator, class Handler>
struct RequestRepeater
{
#if defined(AGRPC_STANDALONE_ASIO) || defined(AGRPC_BOOST_ASIO)
    using executor_type = asio::associated_executor_t<Handler>;
    using allocator_type = asio::associated_allocator_t<Handler>;
#endif

    RPC rpc;
    Service& service;
    detail::AllocatedPointer<RPCHandlerAllocator> rpc_handler;
    Handler handler;

    RequestRepeater(RPC rpc, Service& service, detail::AllocatedPointer<RPCHandlerAllocator> rpc_handler,
                    Handler handler)
        : rpc(rpc), service(service), rpc_handler(std::move(rpc_handler)), handler(std::move(handler))
    {
    }

    void operator()(bool ok);

#if defined(AGRPC_STANDALONE_ASIO) || defined(AGRPC_BOOST_ASIO)
    executor_type get_executor() const noexcept { return asio::get_associated_executor(handler); }

    allocator_type get_allocator() const noexcept { return asio::get_associated_allocator(handler); }
#else
    void set_done() noexcept {}

    void set_value(bool ok) { (*this)(ok); }

    void set_error(std::exception_ptr) noexcept {}

    friend auto tag_invoke(unifex::tag_t<unifex::get_allocator>, const RequestRepeater& self) noexcept
    {
        return detail::get_allocator(self.handler);
    }
#endif
};

template <class Allocator>
struct NoOpReceiverWithAllocator
{
    using allocator_type = Allocator;

    Allocator allocator;

    constexpr explicit NoOpReceiverWithAllocator(Allocator allocator) noexcept(
        std::is_nothrow_copy_constructible_v<Allocator>)
        : allocator(allocator)
    {
    }

    static constexpr void set_done() noexcept {}

    template <class... Args>
    static constexpr void set_value(Args&&...) noexcept
    {
    }

    static void set_error(std::exception_ptr) noexcept {}

    constexpr auto get_allocator() const noexcept { return allocator; }

#ifdef AGRPC_UNIFEX
    friend constexpr auto tag_invoke(unifex::tag_t<unifex::get_allocator>,
                                     const NoOpReceiverWithAllocator& receiver) noexcept
    {
        return receiver.allocator;
    }
#endif
};

template <class RPC, class Service, class Handler>
void submit_request_repeat(RPC rpc, Service& service, Handler handler)
{
    const auto allocator = detail::get_allocator(handler);
    detail::submit(agrpc::repeatedly_request(rpc, service, std::move(handler)),
                   detail::NoOpReceiverWithAllocator{allocator});
}

template <class RPC, class Service, class Request, class Responder, class Handler>
auto RepeatedlyRequestFn::operator()(detail::ServerMultiArgRequest<RPC, Request, Responder> rpc, Service& service,
                                     Handler handler) const
{
#if defined(AGRPC_STANDALONE_ASIO) || defined(AGRPC_BOOST_ASIO)
    auto rpc_handler = detail::allocate<detail::MultiArgRPCContext<Request, Responder>>(detail::get_allocator(handler));
    auto& rpc_context = rpc_handler->server_context();
    auto& rpc_request = rpc_handler->request();
    auto& rpc_responder = rpc_handler->responder();
    agrpc::request(rpc, service, rpc_context, rpc_request, rpc_responder,
                   detail::RequestRepeater{rpc, service, std::move(rpc_handler), std::move(handler)});
#else
    return unifex::let_value_with(
        [&]
        {
            return detail::MultiArgRPCContext<Request, Responder>();
        },
        [&service, rpc, handler = std::move(handler)](auto& context) mutable
        {
            const auto scheduler = detail::get_scheduler(handler);
            return unifex::let_value(agrpc::request(rpc, service, context.server_context(), context.request(),
                                                    context.responder(), agrpc::use_sender(scheduler)),
                                     [&context, &service, rpc, handler = std::move(handler)](bool ok) mutable
                                     {
                                         if AGRPC_LIKELY (ok)
                                         {
                                             detail::submit_request_repeat(rpc, service, handler);
                                         }
                                         return std::move(handler)(context.server_context(), context.request(),
                                                                   context.responder());
                                     });
        });
#endif
}

template <class RPC, class Service, class Responder, class Handler>
auto RepeatedlyRequestFn::operator()(detail::ServerSingleArgRequest<RPC, Responder> rpc, Service& service,
                                     Handler handler) const
{
#if defined(AGRPC_STANDALONE_ASIO) || defined(AGRPC_BOOST_ASIO)
    auto rpc_handler = detail::allocate<detail::SingleArgRPCContext<Responder>>(detail::get_allocator(handler));
    auto& rpc_context = rpc_handler->server_context();
    auto& rpc_responder = rpc_handler->responder();
    agrpc::request(rpc, service, rpc_context, rpc_responder,
                   detail::RequestRepeater{rpc, service, std::move(rpc_handler), std::move(handler)});
#else
    return unifex::let_value_with(
        [&]
        {
            return detail::SingleArgRPCContext<Responder>();
        },
        [&service, rpc, handler = std::move(handler)](auto& context) mutable
        {
            const auto scheduler = detail::get_scheduler(handler);
            return unifex::let_value(agrpc::request(rpc, service, context.server_context(), context.responder(),
                                                    agrpc::use_sender(scheduler)),
                                     [&context, &service, rpc, handler = std::move(handler)](bool ok)
                                     {
                                         if AGRPC_LIKELY (ok)
                                         {
                                             detail::submit_request_repeat(rpc, service, handler);
                                         }
                                         return std::move(handler)(context.server_context(), context.responder());
                                     });
        });
#endif
}

template <class RPC, class Service, class RPCHandler, class Handler>
void RequestRepeater<RPC, Service, RPCHandler, Handler>::operator()(bool ok)
{
    if AGRPC_LIKELY (ok)
    {
        auto next_handler{this->handler};
        agrpc::repeatedly_request(this->rpc, this->service, std::move(next_handler));
    }
    std::move(this->handler)(detail::RepeatedlyRequestContextAccess::create(std::move(this->rpc_handler)), ok);
}
}

AGRPC_NAMESPACE_END

#endif  // AGRPC_DETAIL_REPEATEDLYREQUEST_HPP
