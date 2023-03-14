//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_WAIT_HPP
#define BOOST_ASYNC_DETAIL_WAIT_HPP

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
#include <boost/intrusive_ptr.hpp>
#include <boost/system/result.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>

namespace boost::async::detail
{

struct wait_shared_state
{
  std::unique_ptr<void, coro_deleter<>> h;
  std::size_t use_count = 0u;

  friend void intrusive_ptr_add_ref(wait_shared_state * st) {st->use_count++;}
  friend void intrusive_ptr_release(wait_shared_state * st) {if (st->use_count-- == 1u) st->h.reset();}

  void complete()
  {
    if (use_count == 1u && h != nullptr)
      std::coroutine_handle<void>::from_address(h.release()).resume();
  }

  struct completer
  {
    intrusive_ptr<wait_shared_state> ptr;
    completer(wait_shared_state * wss) : ptr{wss} {}

    void operator()()
    {
      auto p = std::move(ptr);
      if (p->use_count == 1u)
        p.detach()->complete();
      else
        p->complete();
    }
  };

  completer get_completer()
  {
    return {this};
  }
};


template<typename Awaitable>
auto get_resume_result(Awaitable & aw) -> system::result<decltype(aw.await_resume()), std::exception_ptr>
{
  using type = decltype(aw.await_resume());
  try
  {
    if constexpr (std::is_void_v<type>)
    {
      aw.await_resume();
      return {};
    }
    else
      return aw.await_resume();
  }
  catch(...)
  {
    return std::current_exception();
  }
}


template<typename ... Args>
struct wait_variadic_impl
{
  using tuple_type = std::tuple<decltype(get_awaitable_type(std::declval<Args&&>()))...>;

  wait_variadic_impl(Args && ... args)
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

    wait_shared_state wss;

    template<typename T>
    using result_part = system::result<co_await_result_t<T>, std::exception_ptr>;

    std::tuple<result_part<Args> ...> result
        {
          result_part<Args>{system::in_place_error, std::exception_ptr()}...
        };

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};

    template<std::size_t Idx, typename Aw>
    void await_suspend_step(
        asio::io_context::executor_type exec, Aw && aw)
    {
      if (!ready[Idx])
        suspend_for_callback(
          aw,
          asio::bind_cancellation_slot(
            cancel[Idx].slot(),
            asio::bind_executor(
              exec,
              asio::bind_allocator(
                alloc,
                [this, &aw, c = wss.get_completer()]() mutable
                {
                  std::get<Idx>(result) = get_resume_result(aw);
                  c();
                }
              )
            )
          )
        );
      else
        std::get<Idx>(result) = get_resume_result(aw);
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      std::size_t idx = 0u;
      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>
          ([&](auto idx)
          {
            auto & aw = std::get<idx>(aws);
            if constexpr (requires {h.promise().get_executor();})
              await_suspend_step<idx>(h.promise().get_executor(), aw);
            else
              await_suspend_step<idx>(this_thread::get_executor(), aw);
          });
      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      if constexpr (requires {h.promise().get_cancellation_slot();})
        if (h.promise().get_cancellation_slot().is_connected())
          h.promise().get_cancellation_slot().assign(
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
      //return mp11::tuple_transform(get_resume_result{}, aws);
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable(args, std::make_index_sequence<sizeof...(Args)>{});
  }
};


template<typename ... Args>
task<std::tuple<system::result<co_await_result_t<Args>, std::exception_ptr>...>> wait_impl(Args ... args)
{
  std::tuple aws{get_awaitable_type(static_cast<Args&&>(args))...};
  co_return co_await wait_variadic_impl{aws};
}



template<typename Range>
struct wait_ranged_impl
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


    awaitable(Range & aws_, std::false_type /* needs co_await */)
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
    awaitable(Range & aws, std::true_type /* needs co_await */)
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

    wait_shared_state wss;

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};
    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      // default cb
      std::optional<asio::io_context::executor_type> exec;
      if constexpr (requires {h.promise().get_executor();})
        exec = h.promise().get_executor();
      else
        exec = this_thread::get_executor();

      std::size_t idx = 0u;
      for (auto && aw : aws)
      {
        if (!ready[idx])
          suspend_for_callback(
              aw,
              asio::bind_cancellation_slot(
                  cancel[idx].slot(),
                  asio::bind_executor(
                    *exec,
                    asio::bind_allocator(
                      alloc,
                      [this, c = wss.get_completer(), idx]() mutable
                      {
                        result[idx] = get_resume_result(aws[idx]);
                        c();
                      }
                    )
                  )
              )
          );
        else
          result[idx] = get_resume_result(aws[idx]);
        idx ++;
      }
      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      if constexpr (requires {h.promise().get_cancellation_slot();})
        if (h.promise().get_cancellation_slot().is_connected())
        {
          sl = h.promise().get_cancellation_slot();
          if (sl.is_connected())
            sl.assign(
                [&](asio::cancellation_type ct)
                {
                  for (auto & cs : cancel)
                    cs.emit(ct);
                });
        }

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

#endif //BOOST_ASYNC_DETAIL_WAIT_HPP
