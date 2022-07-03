// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_HANDLER_HPP
#define CORO_HANDLER_HPP

#include <coro/this_coro.hpp>
#include <coro/util.hpp>

#include <memory>
#include <optional>

namespace coro
{

namespace detail
{

template<typename Promise>
struct completion_handler_base;

template<typename Promise, typename =  void>
struct completion_handler_cancellation_base
{
};

template<typename Promise>
struct completion_handler_cancellation_base<Promise, std::void_t<typename Promise::cancellation_slot_type>>
{
    using promise_type = Promise;


    using cancellation_slot_type = typename promise_type::cancellation_slot_type;
    cancellation_slot_type get_cancellation_slot() const noexcept
    {
        return static_cast<const completion_handler_base<Promise>*>(this)->self->get_cancellation_slot();
    }
};

template<typename Promise, typename =  void>
struct completion_handler_executor_base
{
};

template<typename Promise>
struct completion_handler_executor_base<Promise, std::void_t<typename Promise::executor_type>>
{
    using promise_type = Promise;


    using executor_type = typename promise_type::executor_type;
    executor_type get_executor() const noexcept
    {
        return static_cast<const completion_handler_base<Promise>*>(this)->self->get_executor();
    }
};

template<typename Promise, typename =  void>
struct completion_handler_allocator_base
{
};

template<typename Promise>
struct completion_handler_allocator_base<Promise, std::void_t<typename Promise::allocator_type>>
{
    using promise_type = Promise;


    using allocator_type = typename promise_type::allocator_type;
    allocator_type get_allocator() const noexcept
    {
        return static_cast<const completion_handler_base<Promise>*>(this)->self->get_allocator();
    }
};

template<typename Promise>
struct completion_handler_base
        : completion_handler_cancellation_base<Promise>,
          completion_handler_executor_base<Promise>,
          completion_handler_allocator_base<Promise>
{
    std::unique_ptr<Promise, coro_deleter<Promise>> self;
    completion_handler_base(std::coroutine_handle<Promise> h) : self(&h.promise(), coro_deleter<Promise>{}) {}
    completion_handler_base(completion_handler_base && ) = default;
};


}

template<typename Promise, typename ... Args>
struct completion_handler : detail::completion_handler_base<Promise>
{
    completion_handler(std::coroutine_handle<Promise> h, std::optional<std::tuple<Args...>> &result)
            : detail::completion_handler_base<Promise>(h), result(result) {}
    completion_handler(completion_handler && ) = default;

    std::optional<std::tuple<Args...>> &result;
    using promise_type = Promise;

    void operator()(Args ... args)
    {
        if constexpr (requires (promise_type & p) {p.notify_suspended();})
            this->self->notify_suspended();

        result.emplace(std::move(args)...);
        std::coroutine_handle<promise_type>::from_promise(*this->self.release()).resume();
    }
};

inline void interpret_result(std::tuple<> && args)
{
}

template<typename ... Args>
auto interpret_result(std::tuple<Args...> && args)
{
    return std::move(args);
}

template<typename ... Args>
auto interpret_result(std::tuple<std::exception_ptr, Args...> && args)
{
    if (std::get<0>(args))
        std::rethrow_exception(std::get<0>(args));
    return std::apply([](auto first, auto && ... rest) {return std::make_tuple(std::move(rest)...);});
}

template<typename ... Args>
auto interpret_result(std::tuple<asio::error_code, Args...> && args)
{
    if (std::get<0>(args))
        throw asio::system_error(std::get<0>(args));
    return std::apply([](auto first, auto && ... rest) {return std::make_tuple(std::move(rest)...);});
}

template<typename  Arg>
auto interpret_result(std::tuple<Arg> && args)
{
    return std::get<0>(std::move(args));
}

template<typename Arg>
auto interpret_result(std::tuple<std::exception_ptr, Arg> && args)
{
    if (std::get<0>(args))
        std::rethrow_exception(std::get<0>(args));
    return std::get<1>(std::move(args));
}

inline auto interpret_result(std::tuple<asio::error_code> && args)
{
    if (std::get<0>(args))
        throw asio::system_error(std::get<0>(args));
}


inline auto interpret_result(std::tuple<std::exception_ptr> && args)
{
    if (std::get<0>(args))
        std::rethrow_exception(std::get<0>(args));
}

template<typename Arg>
auto interpret_result(std::tuple<asio::error_code, Arg> && args)
{
    if (std::get<0>(args))
        throw asio::system_error(std::get<0>(args));
    return std::get<1>(std::move(args));
}


inline void forward_result(std::tuple<> && args)
{
}

template<typename Arg>
auto forward_result(std::tuple<Arg> && arg)
{
    return std::get<0>(std::move(arg));
}

template<typename ... Args>
auto forward_result(std::tuple<Args...> && args)
{
    return std::move(args);
}


}

#endif //CORO_HANDLER_HPP
