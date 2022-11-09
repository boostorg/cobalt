// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_HANDLER_HPP
#define BOOST_ASYNC_HANDLER_HPP

#include <boost/asio/io_context.hpp>

#include "boost/async/this_coro.hpp"
#include "boost/async/detail/util.hpp"

#include <memory>
#include <boost/container/pmr/memory_resource.hpp>
#include <optional>

namespace boost::async::detail
{
    
struct completion_handler_base
{
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type cancellation_slot ;
  cancellation_slot_type get_cancellation_slot() const noexcept
  {
      return cancellation_slot ;
  }

  using executor_type = asio::io_context::executor_type;
  executor_type executor ;
  executor_type get_executor() const noexcept
  {
    return executor ;
  }

  using allocator_type = container::pmr::polymorphic_allocator<void>;
  allocator_type allocator ;
  allocator_type get_allocator() const noexcept
  {
    return allocator ;
  }

  template<typename Promise>
  completion_handler_base(std::coroutine_handle<Promise> h)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor(asio::get_associated_executor(h.promise(), this_thread::get_executor())),
            allocator(asio::get_associated_allocator(h.promise(), this_thread::get_allocator())) {}
};

template<typename Handler,typename ... Args>
struct completion_handler_wrapper
{
  struct promise_type : promise_memory_resource_base
  {
    Handler &handler;
    std::optional<std::tuple<Args...>> & res;
    using cancellation_slot_type = asio::cancellation_slot;
    cancellation_slot_type get_cancellation_slot() const noexcept
    {
      return asio::get_associated_cancellation_slot(handler) ;
    }

    using executor_type = asio::io_context::executor_type;
    executor_type get_executor() const noexcept
    {
      return asio::get_associated_executor(handler, this_thread::get_executor()) ;
    }

    using allocator_type = container::pmr::polymorphic_allocator<void>;
    allocator_type get_allocator() const noexcept
    {
      return asio::get_associated_allocator(handler, this_thread::get_allocator()) ;
    }

    promise_type(Handler & handler,
                 std::optional<std::tuple<Args...>> & res) : handler(handler), res(res) {}



    std::suspend_always initial_suspend() {return {};}
    std::suspend_never final_suspend() noexcept {return {};}

    void unhandled_exception() {throw;}

    completion_handler_wrapper get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }

    void return_void() {}
  };

  std::coroutine_handle<promise_type> promise;

  static auto run(Handler handler, std::optional<std::tuple<Args...>> res = std::nullopt) -> completion_handler_wrapper<Handler, Args...>
  {
    std::apply(std::move(handler), std::move(*res));
    co_return ;
  }

};

template<typename ... Args>
struct completion_handler : completion_handler_base
{
    std::unique_ptr<void, coro_deleter<void>> self;
    std::optional<std::tuple<Args...>> &result;

    void operator()(Args ... args)
    {
        if (notify_suspended_impl)
            notify_suspended_impl(self.get());

        result.emplace(std::move(args)...);
        auto p = this->self.release();
        std::coroutine_handle<void>::from_address(p).resume();
    }

    void  (*notify_suspended_impl)(void*) = nullptr;

    completion_handler(completion_handler && ) = default;

    template<typename Promise>
    completion_handler(std::coroutine_handle<Promise> h, std::optional<std::tuple<Args...>> &result)
            : completion_handler_base(h),
              self(h.address(), coro_deleter<void>{h}), result(result)

    {
        if constexpr (requires (Promise & p) {p.notify_suspended();})
            notify_suspended_impl = +[](void * p) {std::coroutine_handle<Promise>::from_address(p).promise().notify_suspended(); };
    }

    template<typename Handler>
    completion_handler(completion_handler_wrapper<Handler, Args...> w)
            : completion_handler(w.promise, w.promise.promise().res)
    {}

    template<std::invocable<Args...> CompletionHandler>
    completion_handler(CompletionHandler && ch)
      : completion_handler(completion_handler_wrapper<std::decay_t<CompletionHandler>, Args...>::run(std::forward<CompletionHandler>(ch)))
    {

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
auto interpret_result(std::tuple<system::error_code, Args...> && args)
{
    if (std::get<0>(args))
        throw system::system_error(std::get<0>(args));
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

inline auto interpret_result(std::tuple<system::error_code> && args)
{
    if (std::get<0>(args))
        throw system::system_error(std::get<0>(args));
}


inline auto interpret_result(std::tuple<std::exception_ptr> && args)
{
    if (std::get<0>(args))
        std::rethrow_exception(std::get<0>(args));
}

template<typename Arg>
auto interpret_result(std::tuple<system::error_code, Arg> && args)
{
    if (std::get<0>(args))
        throw system::system_error(std::get<0>(args));
    return std::get<1>(std::move(args));
}

}

#endif //BOOST_ASYNC_HANDLER_HPP
