// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_COMPOSE_HPP
#define CORO_COMPOSE_HPP

#include <coro/allocator.hpp>
#include <coro/executor.hpp>
#include <coro/ops.hpp>
#include <coro/this_coro.hpp>
#include <coro/util.hpp>

#include <asio/cancellation_state.hpp>
#include <asio/dispatch.hpp>
#include <asio/post.hpp>
#include <asio/error.hpp>

#include <optional>
#include <tuple>
#include <utility>
#include <variant>

namespace coro
{

template<typename ... Signatures>
struct composed_completion;

template<typename ... Args>
struct composed_completion<void(Args...)>
{
    std::optional<std::tuple<Args...>> result;
    void operator()(Args ... args)
    {
        result = {std::move(args)...};
    }

    auto complete() { return interpret_result(std::move(*result)); }
    auto complete(asio::error_code &) { return interpret_result(std::move(*result)); }
};


template<typename ... Args>
struct composed_completion<void(asio::error_code, Args...)>
{
    asio::error_code ec;
    std::optional<std::tuple<Args...>> result;
    void operator()(asio::error_code ec_, Args ... args)
    {
        ec = ec_;
        result = {std::move(args)...};
    }

    auto complete()
    {
        return interpret_result(std::tuple_cat(std::make_tuple(ec), std::move(*result)));
    }

    auto complete(asio::error_code & ec_)
    {
        ec_ = ec;
        return interpret_result(std::move(*result));
    }
};
template<typename C>
struct is_composed_completion : std::false_type {};

template<typename ... Signatures>
struct is_composed_completion<composed_completion<Signatures...>> : std::true_type{};

template<typename Handler, typename Executor, typename ...Signatures>
struct composed_op
{
    using handler_type = Handler;
    using executor_type = Executor;
    constexpr static bool synchronous = is_composed_completion<std::decay_t<Handler>>::value;

    executor_type get_executor() const {return exec;}
    explicit composed_op(Handler&& handler, Executor exec)
            : handler(static_cast<Handler>(handler)), exec(std::move(exec)) {}

    template<typename, typename , typename ...>
    friend struct composed_promise_sync;

    template<typename, typename, typename ...>
    friend struct composed_promise;

    template<typename>
    friend struct composed_promise_allocator;

  private:
    Handler handler;
    Executor exec;
};



template<typename Handler>
struct composed_promise_allocator
{

    template<typename ... Args>
    void * operator new(const std::size_t size, Args & ... args)
    {
        return allocate_coroutine(size, asio::get_associated_allocator(get_last_variadic(args...).handler));
    }

    void operator delete(void * raw, const std::size_t size)
    {
        deallocate_coroutine<allocator_type>(raw, size);
    }


    using allocator_type = asio::associated_allocator_t<Handler>;
    constexpr allocator_type get_allocator() const {return alloc_;}

    composed_promise_allocator(allocator_type alloc) : alloc_(std::move(alloc)) {}
  private:
    allocator_type alloc_;
};

template<typename Handler>
    requires (std::is_same_v<asio::associated_allocator_t<Handler>, std::allocator<void>>)
struct composed_promise_allocator<Handler>
{
    composed_promise_allocator(std::allocator<void>) {}
    using allocator_type = asio::associated_allocator_t<Handler>;
    constexpr allocator_type get_allocator() const {return {};}
};





template<typename Derived, typename Signature>
struct composed_promise_base;

template<typename Derived, typename ... Args>
struct composed_promise_base<Derived, void(Args...)>
{
    void return_value(std::tuple<Args...>  args)
    {
        static_cast<Derived*>(this)->result_ = std::move(args);
    }

    using tuple_type = std::tuple<Args...>;
};

template<typename Handler, typename Executor, typename ...Signatures>
struct composed_promise
      : promise_cancellation_base<asio::cancellation_slot, asio::enable_terminal_cancellation>,
        promise_throw_if_cancelled_base,
        promise_executor_base<Executor>,
        composed_promise_allocator<Handler>,
        enable_await_allocator<composed_promise<Handler, Executor, Signatures...>>,
        enable_operations<false>,
        composed_promise_base<composed_promise<Handler, Executor, Signatures...>, Signatures> ...
{
    using my_type = composed_promise<Handler, Executor, Signatures...>;

    composed_promise(composed_op<Handler, Executor, Signatures...> & op) :
            promise_cancellation_base<asio::cancellation_slot, asio::enable_terminal_cancellation>(asio::get_associated_cancellation_slot(op.handler)),
            promise_executor_base<Executor>(op.get_executor()),
            composed_promise_allocator<Handler>(asio::get_associated_allocator(op.handler)),
            completion_handler(static_cast<Handler>(op.handler)) {}

    template<typename ... Args>
    composed_promise(Args & ... args) : composed_promise(get_last_variadic(args...)) {}

    using promise_cancellation_base<asio::cancellation_slot, asio::enable_terminal_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using promise_executor_base<Executor>::await_transform;
    using enable_await_allocator<composed_promise<Handler, Executor, Signatures...>>::await_transform;
    using enable_operations<false>::await_transform;

    using completion_handler_type = Handler;
    completion_handler_type completion_handler;

    using result_type = std::variant<typename composed_promise_base<my_type, Signatures>::tuple_type ...>;
    std::optional<result_type> result_;

    constexpr static std::suspend_never initial_suspend() noexcept { return {}; }
    constexpr static auto final_suspend() noexcept
    {
        struct completion
        {
            constexpr static bool await_ready() noexcept {return false;}
            void await_suspend(std::coroutine_handle<composed_promise> h) noexcept
            {
                auto & pro = h.promise();
                auto exec = pro.get_executor();
                auto sus = pro.did_suspend;
                struct cc
                {
                    Handler cpl;
                    result_type res;


                    void operator()()
                    {
                        std::visit([&](auto && tup )
                                   {
                                       return std::apply(std::move(cpl),
                                                         std::move(tup));
                                   }, std::move(res));
                    }
                } cc_{
                        static_cast<Handler>(pro.completion_handler),
                        std::move(pro.result_.value())
                    };

                h.destroy();
                if (sus)
                    asio::dispatch(exec, std::move(cc_));
                else
                    asio::post(exec, std::move(cc_));
            }

            constexpr static void await_resume() noexcept {}
        };

        return completion{};
    }

    bool did_suspend = false;
    void notify_suspended() noexcept { did_suspend = true;}

    constexpr void get_return_object() {}
    void unhandled_exception()
    {
        if (did_suspend)
            std::coroutine_handle<composed_promise>::from_promise(*this).destroy();
        throw ;
    }
};

template<typename Handler, typename Executor, typename ...Signatures>
struct composed_promise_sync
    : promise_cancellation_base<asio::cancellation_slot, asio::enable_terminal_cancellation>,
      promise_executor_base<Executor>,
      enable_await_allocator<composed_promise_sync<Handler, Executor, Signatures...>>,
      enable_operations<true>,
      composed_promise_base<composed_promise_sync<Handler, Executor, Signatures...>, Signatures> ...

{
    using my_type = composed_promise_sync<Handler, Executor, Signatures...>;
    using promise_cancellation_base<asio::cancellation_slot, asio::enable_terminal_cancellation>::await_transform;
    using promise_executor_base<Executor>::await_transform;
    using enable_operations<true>::await_transform;
    using enable_await_allocator<composed_promise_sync<Handler, Executor, Signatures...>>::await_transform;

    using completion_handler_type = Handler;
    completion_handler_type completion_handler;

    using result_type = std::variant<typename composed_promise_base<my_type, Signatures>::tuple_type ...>;
    std::optional<result_type> result_;

    composed_promise_sync(composed_op<Handler, Executor, Signatures...> & op) :
            promise_executor_base<Executor>(op.get_executor()),
            completion_handler(static_cast<Handler>(op.handler)) {}

    template<typename ... Args>
    composed_promise_sync(Args & ... args) : composed_promise_sync(get_last_variadic(args...)) {}


    constexpr static std::suspend_never initial_suspend() noexcept { return {}; }
    auto final_suspend() noexcept
    {
        struct completion
        {
            constexpr static bool await_ready() noexcept {return false;}
            void await_suspend(std::coroutine_handle<composed_promise_sync> h) noexcept
            {
                auto & pro = h.promise();
                Handler cpl = static_cast<Handler>(pro.completion_handler);
                auto res = std::move(pro.result_.value());
                h.destroy();
                std::visit([&](auto && tup )
                           {
                                return std::apply(std::move(cpl),
                                                  std::move(tup));
                            }, std::move(res));
            }

            constexpr static void await_resume() noexcept {}
        };

        return completion{};
    }

    bool did_suspend = false;
    void notify_suspended() noexcept { did_suspend = true;}

    using allocator_type = std::allocator<void>;
    constexpr allocator_type get_allocator() const {return std::allocator<void>();}

    constexpr void get_return_object() {}
    void unhandled_exception()
    {
        throw ;
    }
};


template<typename C>
struct is_composed_op : std::false_type {};

template<typename Handler, typename Executor, typename ... Signatures>
struct is_composed_op<composed_op<Handler, Executor, Signatures...>> : std::true_type
{
    using promise_type =
            std::conditional_t<composed_op<Handler, Executor, Signatures...>::synchronous,
            coro::composed_promise_sync<Handler, Executor, Signatures...>,
            coro::composed_promise<Handler, Executor, Signatures...>>;
};


}

namespace std
{

template<typename ... Args>
    requires coro::is_composed_op<coro::variadic_last_t<Args...>>::value
struct coroutine_traits<void, Args...>
{
    using promise_type = typename coro::is_composed_op<coro::variadic_last_t<Args...>>::promise_type;
};

}

namespace coro
{

namespace detail
{

template<typename Implementation, typename ... Signatures>
struct composed_initiate
{
    Implementation impl;

    template<typename Completion, typename ... Args>
    void operator()(Completion && cpl, Args && ... args)
    {
        auto exec = detail::pick_executor(0, args...);
        impl(std::forward<Args>(args)...,
             composed_op<Completion, decltype(exec), Signatures...>{std::forward<Completion>(cpl), std::move(exec)});
    }
};

}

template<typename CompletionToken, typename ... Signatures,
         typename Implementation, typename ... IoObjectsOrExecutors>
auto async_compose(Implementation && implementation,
                   CompletionToken& token,
                   IoObjectsOrExecutors && ... io_objects_or_executors)
{
    return asio::async_initiate<CompletionToken, Signatures...>(
            detail::composed_initiate<Implementation, Signatures...>{std::forward<Implementation>(implementation)},
            token, std::forward<IoObjectsOrExecutors>(io_objects_or_executors)...
            );
}

template<typename ... Signatures,
        typename Implementation, typename ... IoObjectsOrExecutors>
auto compose(Implementation && implementation,
             IoObjectsOrExecutors && ... io_objects_or_executors)
{
    composed_completion<Signatures...> cpl;
    auto exec = detail::pick_executor(0, io_objects_or_executors...);
    implementation(
            std::forward<IoObjectsOrExecutors>(io_objects_or_executors)...,
            composed_op<composed_completion<Signatures...>&, decltype(exec), Signatures...>{cpl, std::move(exec)});
    return cpl.complete();
}

template<typename ... Signatures,
        typename Implementation, typename ... IoObjectsOrExecutors>
auto compose(Implementation && implementation,
             asio::error_code & ec,
             IoObjectsOrExecutors && ... io_objects_or_executors)
{
    composed_completion<Signatures...> cpl;
    auto exec = detail::pick_executor(0, io_objects_or_executors...);
    implementation(
            std::forward<IoObjectsOrExecutors>(io_objects_or_executors)...,
            composed_op<composed_completion<Signatures...>&, decltype(exec), Signatures...>{cpl, std::move(exec)});
    return cpl.complete(ec);
}


}


#endif //CORO_COMPOSE_HPP
