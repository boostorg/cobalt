// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_HANDLER_HPP
#define BOOST_ASYNC_HANDLER_HPP

#include <boost/async/this_coro.hpp>
#include <boost/async/unique_handle.hpp>
#include <boost/async/detail/util.hpp>

#if defined(BOOST_ASYNC_NO_PMR)
#include <boost/async/detail/monotonic_resource.hpp>
#endif


#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/post.hpp>

#include <memory>
#include <optional>

namespace boost::async
{

namespace detail
{

enum class completed_immediately_t
{
  no, maybe, yes, initiating
};

struct completion_handler_noop_executor : executor
{
  completed_immediately_t * completed_immediately = nullptr;

  template<typename Fn>
  void execute(Fn && fn) const
  {
    // only allow it when we're still initializing
    if (completed_immediately &&
        ((*completed_immediately == completed_immediately_t::initiating)
      || (*completed_immediately == completed_immediately_t::maybe)))
    {
      // only use this indicator if the fn will actually call our completion-handler
      // otherwise this was a single op in a composed operation
      *completed_immediately = completed_immediately_t::maybe;
      fn();
      // yes means completion_handler::operator() was called, so we're good.
      if (*completed_immediately != completed_immediately_t::yes)
        *completed_immediately = completed_immediately_t::initiating;
    }
    else
    {
      asio::post(*this, std::forward<Fn>(fn));
    }
  }

  friend bool operator==(const completion_handler_noop_executor&, const completion_handler_noop_executor&) noexcept
  {
    return true;
  }

  friend bool operator!=(const completion_handler_noop_executor&, const completion_handler_noop_executor&) noexcept
  {
    return false;
  }

  completion_handler_noop_executor(const completion_handler_noop_executor & rhs) noexcept = default;
  completion_handler_noop_executor(async::executor inner, completed_immediately_t * completed_immediately)
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

#if !defined(BOOST_ASYNC_NO_PMR)
  using allocator_type = pmr::polymorphic_allocator<void>;
  pmr::polymorphic_allocator<void> allocator ;
  allocator_type get_allocator() const noexcept
  {
    return allocator ;
  }
#elif !defined(BOOST_ASYNC_NO_MONOTONIC)
  using allocator_type = detail::monotonic_allocator<void>;
  detail::monotonic_allocator<void> allocator ;
  allocator_type get_allocator() const noexcept
  {
    return allocator ;
  }
#endif
  using immediate_executor_type = completion_handler_noop_executor;
  completed_immediately_t * completed_immediately = nullptr;
  immediate_executor_type get_immediate_executor() const noexcept
  {
    return {get_executor(), completed_immediately};
  }

  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h,
                          completed_immediately_t * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
#if !defined(BOOST_ASYNC_NO_PMR)
            allocator(asio::get_associated_allocator(h.promise(), this_thread::get_allocator())),
#endif
            completed_immediately(completed_immediately)
  {
  }
#if !defined(BOOST_ASYNC_NO_PMR)
  template<typename Promise>
    requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h,
                          pmr::memory_resource * resource,
                          completed_immediately_t * completed_immediately = nullptr)
          : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
            executor_(h.promise().get_executor()),
            allocator(resource),
            completed_immediately(completed_immediately)
  {
  }
#elif !defined(BOOST_ASYNC_NO_MONOTONIC)
  template<typename Promise>
  requires (requires (Promise p) {{p.get_executor()} -> std::same_as<const executor&>;})
  completion_handler_base(std::coroutine_handle<Promise> h,
                          detail::monotonic_resource * resource,
                          completed_immediately_t * completed_immediately = nullptr)
      : cancellation_slot(asio::get_associated_cancellation_slot(h.promise())),
        executor_(h.promise().get_executor()),
        allocator(resource),
        completed_immediately(completed_immediately)
  {
  }

#endif
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
                       detail::completed_immediately_t * completed_immediately = nullptr)
            : completion_handler_base(h, completed_immediately),
              self(h.address()), result(result)
    {
    }

#if !defined(BOOST_ASYNC_NO_PMR)
    template<typename Promise>
    completion_handler(std::coroutine_handle<Promise> h,
                       std::optional<std::tuple<Args...>> &result,
                       pmr::memory_resource * resource,
                       detail::completed_immediately_t * completed_immediately = nullptr)
            : completion_handler_base(h, resource, completed_immediately),
              self(h.address()), result(result)
    {
    }
#elif !defined(BOOST_ASYNC_NO_MONOTONIC)
    template<typename Promise>
    completion_handler(std::coroutine_handle<Promise> h,
                       std::optional<std::tuple<Args...>> &result,
                       detail::monotonic_resource * resource,
                       detail::completed_immediately_t * completed_immediately = nullptr)
        : completion_handler_base(h, resource, completed_immediately),
          self(h.address()), result(result)
    {
    }
#endif


    void operator()(Args ... args)
    {
        result.emplace(std::move(args)...);
        BOOST_ASSERT(this->self != nullptr);
        auto p = this->self.release();
        if (completed_immediately != nullptr
        && *completed_immediately == detail::completed_immediately_t::maybe)
        {
          *completed_immediately = detail::completed_immediately_t::yes;
          return;
        }

        std::move(p)();
    }
    using result_type = std::optional<std::tuple<Args...>>;

    ~completion_handler()
    {
      if (self && completed_immediately
        && *completed_immediately == detail::completed_immediately_t::initiating
        && std::uncaught_exceptions() > 0)
        self.release();
    }
 private:
    unique_handle<void> self;
    std::optional<std::tuple<Args...>> &result;
};



inline void interpret_result(std::tuple<> &&)
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
    return std::apply([](auto, auto && ... rest) {return std::make_tuple(std::move(rest)...);});
}

template<typename ... Args>
auto interpret_result(std::tuple<system::error_code, Args...> && args)
{
    if (std::get<0>(args))
        throw system::system_error(std::get<0>(args));
    return std::apply([](auto, auto && ... rest) {return std::make_tuple(std::move(rest)...);});
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
