// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_ASYNC_OPERATION_HPP
#define BOOST_ASYNC_ASYNC_OPERATION_HPP

#include <boost/async/detail/handler.hpp>

#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/post.hpp>

#include <coroutine>


namespace boost::async::detail
{

template<typename T>
concept async_operation = asio::async_operation<std::decay_t<T>>;

template<typename Signature, typename Op>
struct awaitable_async_operation;

template<typename ... Args, typename Op>
struct awaitable_async_operation<void(Args...), Op>
{
    Op op;
    std::optional<std::tuple<Args...>> result;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    inline void await_suspend( std::coroutine_handle<Promise> h)
    {
        auto exec = asio::get_associated_executor(h.promise());
        try
        {
            using completion = completion_handler<Args...>;
            static_cast<Op>(op)(completion{h, result});
        }
        catch(...)
        {
            asio::post(exec, [e = std::current_exception()]{std::rethrow_exception(e);});
        }
    }

    [[nodiscard]] auto await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
        return interpret_result(std::move(result.value()), loc);
    }
};

struct enable_async_operation
{
    template<async_operation Op>
    auto await_transform(Op && op)
    {
        using signature_type = typename decltype(std::forward<Op>(op)(asio::detail::completion_signature_probe{}))::type;
        return awaitable_async_operation<signature_type, Op>{std::forward<Op>(op)};
    }
};

}

#endif //BOOST_ASYNC_ASYNC_OPERATION_HPP
