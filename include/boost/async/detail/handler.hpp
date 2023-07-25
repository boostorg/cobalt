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

  template<typename Fn>
  void execute(Fn && fn) const
  {
    if (completed_immediately)
    {
      *completed_immediately = true;
      fn();
    }
    else
    {
      asio::post(*this, std::forward<Fn>(fn));
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

  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h, bool * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
            allocator(asio::get_associated_allocator(h.promise(), this_thread::get_allocator())),
            completed_immediately(completed_immediately)
  {
  }

  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h,
                          pmr::memory_resource * resource,
                          bool * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
            allocator(resource),
            completed_immediately(completed_immediately)
  {
  }

};



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
        BOOST_ASSERT(completed_immediately);
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
