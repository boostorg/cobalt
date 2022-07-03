// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_TE_HPP
#define CORO_TE_HPP

#include <coro/handler.hpp>
#include <asio/associated_executor.hpp>
#include <asio/associated_cancellation_slot.hpp>

namespace coro
{

template<typename Signature,
        typename Executor = asio::any_io_executor,
        typename Allocator = std::allocator<void>,
        typename CancellationSlot = asio::cancellation_slot>
struct te_completion_handler;

template<typename ... Args,
         typename Executor,
         typename Allocator,
         typename CancellationSlot>
struct te_completion_handler<void(Args...), Executor, Allocator, CancellationSlot>
{
    std::unique_ptr<void, coro_deleter<void>> self;
    void (*notify_suspended)(void*) = nullptr;

    template<typename Promise>
    static auto make_ns_ptr() -> void(*)(void*)
    {
        if constexpr (requires (Promise & p) {p.notify_suspended();})
            return  +[](void * p) {std::coroutine_handle<Promise>::from_address(p).notify_suspended();};
        else
            return nullptr;
    }

    template<typename Promise>
    te_completion_handler(std::coroutine_handle<Promise> h, std::optional<std::tuple<Args...>> &result)
            : self(h.address()),
              notify_suspended(make_ns_ptr<Promise>()),
              result(result),
              executor_ (asio::get_associated_allocator(h.promise())),
              allocator_(asio::get_associated_executor(h.promise())),
              cancellation_slot_(asio::get_associated_cancellation_slot(h.promise())) {}

    template<typename Promise>
    te_completion_handler(completion_handler<Promise, Args...> && ch)
            : self(ch.self.release()),
              notify_suspended(make_ns_ptr<Promise>()),
              result(ch.result),
              executor_(asio::get_associated_allocator(ch)),
              allocator_(asio::get_associated_executor(ch)),
              cancellation_slot_(asio::get_associated_cancellation_slot(ch)) {}

    te_completion_handler(te_completion_handler && ) = default;


    std::optional<std::tuple<Args...>> &result;

    void operator()(Args ... args)
    {
        if (notify_suspended)
            notify_suspended(self.get());
        result = {std::move(args)...};
        std::coroutine_handle<void>::from_address(*this->self.release()).resume();
    }

    using cancellation_slot_type = CancellationSlot;
    cancellation_slot_type get_cancellation_slot() const noexcept
    {
        return cancellation_slot_;
    }

    using executor_type = Executor;
    executor_type get_executor() const noexcept
    {
        return executor_;
    }

    using allocator_type = Allocator;
    allocator_type get_allocator() const noexcept
    {
        return allocator_;
    }

    executor_type executor_;
    allocator_type allocator_;
    cancellation_slot_type cancellation_slot_;
};

template<typename Signature,
        typename Executor,
        typename Allocator,
        typename CancellationSlot,
        typename ...Init>
struct basic_te_operation;

template<typename ... Args,
        typename Executor,
        typename Allocator,
        typename CancellationSlot,
        typename ...Init>
struct basic_te_operation<void(Args...), Executor, Allocator, CancellationSlot, Init...>
{
    using completion = te_completion_handler<void(Args...), Executor, Allocator, CancellationSlot>;
    std::tuple<Init...> init_args;
    void(*init)(Init..., completion);
    std::optional<std::tuple<Args...>> result;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        detail::throw_if_cancelled_impl(h.promise());
        return std::apply(
                [this, h](auto && ... args)
                {
                    return init(std::move(args)..., completion{h, result});
                }, std::move(init_args));
    }

    auto await_resume()
    {
        return interpret_result(std::move(*result));
    }
};

template<typename Signature, typename ...Inits>
using te_operation = basic_te_operation<Signature,
                                        asio::any_io_executor,
                                        std::allocator<void>,
                                        asio::cancellation_slot, Inits...>;

}

#endif //CORO_TE_HPP
