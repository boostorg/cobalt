// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_HANDLER_HPP
#define BOOST_ASYNC_HANDLER_HPP

#include <boost/async/this_coro.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/post.hpp>

#include <memory>
#include <optional>

namespace boost::async
{

namespace detail
{

struct completion_handler_noop_executor : executor
{
  bool * completed_immediately = nullptr;

  template <typename Function, typename Allocator>
  void dispatch(Function && f, const Allocator& a) const
  {
    if (completed_immediately == nullptr)
      asio::post(*this,  asio::bind_allocator(a, std::forward<Function>(f)));
    else
    {
      *completed_immediately = true;
      std::forward<Function>(f)();
    }
  }

  friend bool operator==(const completion_handler_noop_executor& a, const completion_handler_noop_executor& b) noexcept
  {
    return true;
  }

  friend bool operator!=(const completion_handler_noop_executor& a, const completion_handler_noop_executor& b) noexcept
  {
    return false;
  }

  completion_handler_noop_executor(const completion_handler_noop_executor & rhs) noexcept = default;
  completion_handler_noop_executor(async::executor inner, bool * completed_immediately)
        : async::executor(std::move(inner)), completed_immediately(completed_immediately)
  {
  }

};


struct completion_handler_base
{
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type cancellation_slot ;
  cancellation_slot_type get_cancellation_slot() const noexcept
  {
      return cancellation_slot ;
  }

  using executor_type = executor;
  const executor_type & executor_ ;
  const executor_type & get_executor() const noexcept
  {
    return executor_ ;
  }

  using allocator_type = pmr::polymorphic_allocator<void>;
  pmr::polymorphic_allocator<void> allocator ;
  allocator_type get_allocator() const noexcept
  {
    return allocator ;
  }

  using immediate_executor_type = completion_handler_noop_executor;
  bool * completed_immediately = nullptr;
  immediate_executor_type get_immediate_executor() const noexcept
  {
    return {get_executor(), completed_immediately};
  }

  completion_handler_base(
      cancellation_slot_type cancellation_slot,
      const executor_type & executor,
      allocator_type allocator)
          : cancellation_slot(cancellation_slot)
          , executor_(executor)
          , allocator(allocator)
  {
  }

  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h, bool * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
            allocator(asio::get_associated_allocator(h.promise(), this_thread::get_allocator())),
            completed_immediately(completed_immediately)

  {}

  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h,
                          pmr::memory_resource * resource,
                          bool * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
            allocator(resource),
            completed_immediately(completed_immediately) {}

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

    using executor_type = executor;
    executor_type get_executor() const noexcept
    {
      return asio::get_associated_executor(handler, this_thread::get_executor()) ;
    }

    using allocator_type = pmr::polymorphic_allocator<void>;
    allocator_type get_allocator() const noexcept
    {
      return asio::get_associated_allocator(handler, this_thread::get_allocator()) ;
    }

    using immediate_executor_type = executor_type;
    immediate_executor_type get_immediate_executor() const noexcept
    {
      return asio::get_associated_immediate_executor(handler, this_thread::get_executor());
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

  static auto run(Handler handler,
                  std::optional<std::tuple<Args...>> res = std::nullopt) -> completion_handler_wrapper<Handler, Args...>
  {
    std::apply(std::move(handler), std::move(*res));
    co_return ;
  }

};


template<typename Func>
struct bound_completion_handler : completion_handler_base, Func
{
  using Func::operator();
  template<typename Func_>
  bound_completion_handler(
      completion_handler_base::cancellation_slot_type cancellation_slot,
      const completion_handler_base::executor_type & executor,
      completion_handler_base::allocator_type allocator,
      Func_ && func) : completion_handler_base(cancellation_slot, executor, allocator),
                      Func(std::forward<Func_>(func)) {}

  template<typename Func_>
  bound_completion_handler(
      completion_handler_base::cancellation_slot_type cancellation_slot,
      completion_handler_base::executor_type && executor,
      completion_handler_base::allocator_type allocator,
      Func_ && func) = delete;
};

template<typename Func>
auto bind_completion_handler(
    completion_handler_base::cancellation_slot_type cancellation_slot,
    const completion_handler_base::executor_type & executor,
    completion_handler_base::allocator_type allocator,
    Func && func) -> bound_completion_handler<std::decay_t<Func>>
{
  return bound_completion_handler<std::decay_t<Func>>(
      cancellation_slot,
      executor,
      allocator,
      std::forward<Func>(func));
}


template<typename Func>
auto bind_completion_handler(
    completion_handler_base::cancellation_slot_type cancellation_slot,
    completion_handler_base::executor_type && executor,
    completion_handler_base::allocator_type allocator,
    Func && func) -> bound_completion_handler<std::decay_t<Func>> = delete;

template<typename Handler>
void assign_cancellation(std::coroutine_handle<void>, Handler &&) {}

template<typename Promise, typename Handler>
void assign_cancellation(std::coroutine_handle<Promise> h, Handler && func)
{
  if constexpr (requires {h.promise().get_cancellation_slot();})
    if (h.promise().get_cancellation_slot().is_connected())
      h.promise().get_cancellation_slot().assign(std::forward<Handler>(func));
}

template<typename Promise>
const executor &
get_executor(std::coroutine_handle<Promise> h)
{
  if constexpr (requires {h.promise().get_executor();})
  {
    static_assert(std::same_as<decltype(h.promise().get_executor()),
                               const executor &>,
                  "for performance reasons, the get_executor function on a promise must return a const reference");
    return h.promise().get_executor();
  }
  else
    return this_thread::get_executor();
}

inline const executor &
get_executor(std::coroutine_handle<>)
{
  return this_thread::get_executor();
}

}

template<typename ... Args>
struct handler
{
  void operator()(Args ... args)
  {
    result.emplace(static_cast<Args>(args)...);
  }
  handler(std::optional<std::tuple<Args...>> &result) : result(result) {}
 private:
  std::optional<std::tuple<Args...>> &result;
};

template<typename ... Args>
handler(std::optional<std::tuple<Args...>> &result) -> handler<Args...>;

template<typename ... Args>
struct completion_handler : detail::completion_handler_base
{
    completion_handler(completion_handler && ) = default;

    template<typename Promise>
    completion_handler(std::coroutine_handle<Promise> h,
                       std::optional<std::tuple<Args...>> &result,
                       bool * completed_immediately = nullptr)
            : completion_handler_base(h, completed_immediately),
              self(h.address()), result(result)

    {
    }

    template<typename Promise>
    completion_handler(std::coroutine_handle<Promise> h,
                       std::optional<std::tuple<Args...>> &result,
                       pmr::memory_resource * resource,
                       bool * completed_immediately = nullptr)
            : completion_handler_base(h, resource, completed_immediately),
              self(h.address()), result(result)
    {
    }


  template<typename Handler>
    completion_handler(detail::completion_handler_wrapper<Handler, Args...> w)
            : completion_handler(w.promise, w.promise.promise().res)
    {}

    template<std::invocable<Args...> CompletionHandler>
    completion_handler(CompletionHandler && ch)
      : completion_handler(detail::completion_handler_wrapper<std::decay_t<CompletionHandler>, Args...>::run(std::forward<CompletionHandler>(ch)))
    {

    }
    void operator()(Args ... args)
    {
        result.emplace(std::move(args)...);
        BOOST_ASSERT(this->self != nullptr);
        auto p = this->self.release();
        if (completed_immediately != nullptr && *completed_immediately)
          return;
        std::coroutine_handle<void>::from_address(p).resume();
    }
    using result_type = std::optional<std::tuple<Args...>>;
 private:
    std::unique_ptr<void, detail::coro_deleter<void>> self;
    std::optional<std::tuple<Args...>> &result;
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
