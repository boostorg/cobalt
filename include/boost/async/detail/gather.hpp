//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_GATHER_HPP
#define BOOST_ASYNC_DETAIL_GATHER_HPP

#include <boost/async/detail/await_result_helper.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/detail/ref.hpp>
#include <boost/async/detail/util.hpp>
#include <boost/async/detail/wrapper.hpp>
#include <boost/async/task.hpp>
#include <boost/async/this_thread.hpp>

#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/system/result.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>

namespace boost::async::detail
{

struct gather_shared_state
{
  std::unique_ptr<void, coro_deleter<>> h;
  std::size_t use_count = 0u;

  friend void intrusive_ptr_add_ref(gather_shared_state * st) {st->use_count++;}
  friend void intrusive_ptr_release(gather_shared_state * st) {if (st->use_count-- == 1u) st->h.reset();}

  void complete()
  {
    if (use_count == 0u && h != nullptr)
      std::coroutine_handle<void>::from_address(h.release()).resume();
  }

  struct completer
  {
    intrusive_ptr<gather_shared_state> ptr;
    completer(gather_shared_state * wss) : ptr{wss} {}

    void operator()()
    {
      auto p = std::move(ptr);
      if (p->use_count == 1u)
      {
        auto pp = p.detach();
        pp->use_count--;
        pp->complete();
      }
      else
        p->complete();
    }
  };

  completer get_completer()
  {
    return {this};
  }
};


template<typename ... Args>
struct gather_variadic_impl
{
  using tuple_type = std::tuple<decltype(get_awaitable_type(std::declval<Args&&>()))...>;

  gather_variadic_impl(Args && ... args)
      : args{std::forward<Args>(args)...}
  {
  }

  std::tuple<Args...> args;

  constexpr static std::size_t tuple_size = sizeof...(Args);

  struct awaitable
  {
    template<std::size_t ... Idx>
    awaitable(std::tuple<Args...> & args, std::index_sequence<Idx...>) :
        aws(awaitable_type_getter<Args>(std::get<Idx>(args))...)
    {
    }

    tuple_type aws;

    std::array<bool, tuple_size> ready{
        std::apply([](auto && ... aw) {return std::array<bool, tuple_size>{aw.await_ready() ... };}, aws)
    };
    std::array<asio::cancellation_signal, tuple_size> cancel;
    char storage[256 * tuple_size];
    container::pmr::monotonic_buffer_resource res{storage, sizeof(storage),
                                                  this_thread::get_default_resource()};
    container::pmr::polymorphic_allocator<void> alloc{&res};

    gather_shared_state wss;

    template<typename T>
    using result_part = system::result<co_await_result_t<T>, std::exception_ptr>;

    std::tuple<result_part<Args> ...> result
        {
          result_part<Args>{system::in_place_error, std::exception_ptr()}...
        };

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};

    template<std::size_t Idx, typename Aw>
    void await_suspend_step(
        const asio::io_context::executor_type & exec,
        Aw && aw)
    {
      if (!ready[Idx])
        suspend_for_callback(
          aw,
          bind_completion_handler(
              cancel[Idx].slot(),
              exec,
              alloc,
              [this, &aw, c = wss.get_completer()]() mutable
              {
                std::get<Idx>(result) = get_resume_result(aw);
                c();
              }
            )
          );
      else
        std::get<Idx>(result) = get_resume_result(aw);
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      auto exec = get_executor(h);
      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>
          ([&](auto idx)
          {
            await_suspend_step<idx>(exec, std::get<idx>(aws));
          });

      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      assign_cancellation(
              h,
              [&](asio::cancellation_type ct)
              {
                for (auto & cs : cancel)
                  cs.emit(ct);
              });

      wss.h.reset(h.address());
      return true;
    }

    auto await_resume()
    {
      wss.h.release();
      return std::move(result);
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable(args, std::make_index_sequence<sizeof...(Args)>{});
  }
};

template<typename Range>
struct gather_ranged_impl
{
  Range aws;

  using result_type = system::result<
      co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<Range>()))>>,
      std::exception_ptr>;

  struct awaitable
  {
    using type = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;
    container::pmr::monotonic_buffer_resource res;
    container::pmr::polymorphic_allocator<void> alloc{&res};

    std::conditional_t<awaitable_type<type>, Range &,
                       container::pmr::vector<co_awaitable_type<type>>> aws;

    container::pmr::vector<bool> ready{std::size(aws), alloc};
    container::pmr::vector<asio::cancellation_signal> cancel{std::size(aws), alloc};
    container::pmr::vector<result_type> result{
          cancel.size(),
          result_type{system::in_place_error, std::exception_ptr()},
          this_thread::get_allocator()};


    awaitable(Range & aws_, std::false_type /* needs operator co_await */)
      : res((256 + sizeof(co_awaitable_type<type>)) * std::size(aws_),
            this_thread::get_default_resource())
      , aws{alloc}
      , ready{std::size(aws_), alloc}
      , cancel{std::size(aws_), alloc}
    {
      aws.reserve(std::size(aws_));
      for (auto && a : aws_)
        aws.emplace_back(awaitable_type_getter<decltype(a)>(std::forward<decltype(a)>(a)));

      std::transform(std::begin(this->aws),
                     std::end(this->aws),
                     std::begin(ready),
                     [](auto & aw) {return aw.await_ready();});
    }
    awaitable(Range & aws, std::true_type /* needs operator co_await */)
        : res((256 + sizeof(co_awaitable_type<type>)) * std::size(aws),
              this_thread::get_default_resource())
        , aws(aws)
    {
      std::transform(std::begin(aws), std::end(aws), std::begin(ready), [](auto & aw) {return aw.await_ready();});
    }

    awaitable(Range & aws)
      : awaitable(aws, std::bool_constant<awaitable_type<type>>{})
    {
    }

    gather_shared_state wss;

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};
    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      // default cb
      auto exec = get_executor(h);

      std::size_t idx = 0u;
      for (auto && aw : aws)
      {
        if (!ready[idx])
          suspend_for_callback(
            aw,
            bind_completion_handler(
              cancel[idx].slot(),
              exec, alloc,
              [this, c = wss.get_completer(), idx]() mutable
              {
                result[idx] = get_resume_result(aws[idx]);
                c();
              }
            )
          );
        else
          result[idx] = get_resume_result(aws[idx]);
        idx ++;
      }
      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      assign_cancellation(
          h,
          [&](asio::cancellation_type ct)
          {
            for (auto & cs : cancel)
              cs.emit(ct);
          });

      wss.h.reset(h.address());
      return true;
    }

    asio::cancellation_slot sl;

    container::pmr::vector<result_type> await_resume()
    {
      sl.clear();
      ignore_unused(wss.h.release());
      return std::move(result);
    }
  };
  awaitable operator co_await() && {return awaitable{aws};}
};

}

namespace boost::async
{

template<typename ... Args>
struct is_interruptible<detail::gather_variadic_impl<Args...>> : std::true_type {};
template<typename Range>
struct is_interruptible<detail::gather_ranged_impl<Range>> : std::true_type {};

}


#endif //BOOST_ASYNC_DETAIL_GATHER_HPP
