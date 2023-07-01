//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_JOIN_HPP
#define BOOST_ASYNC_DETAIL_JOIN_HPP

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

#include <array>
#include <coroutine>

namespace boost::async::detail
{

struct join_shared_state
{
  std::unique_ptr<void, coro_deleter<>> h;
  std::size_t use_count = 0u;

  friend void intrusive_ptr_add_ref(join_shared_state * st) {st->use_count++;}
  friend void intrusive_ptr_release(join_shared_state * st) {if (st->use_count-- == 1u) st->h.reset();}

  void complete()
  {
    if (use_count == 0u && h != nullptr)
      std::coroutine_handle<void>::from_address(h.release()).resume();
  }

  struct completer
  {
    intrusive_ptr<join_shared_state> ptr;
    completer(join_shared_state * wss) : ptr{wss} {}

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
struct join_variadic_impl
{
  using tuple_type = std::tuple<decltype(get_awaitable_type(std::declval<Args&&>()))...>;

  join_variadic_impl(Args && ... args)
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

    std::array<asio::cancellation_signal, tuple_size> cancel_;
    template<typename > constexpr static auto make_null() {return nullptr;};
    std::array<asio::cancellation_signal*, tuple_size> cancel = {make_null<Args>()...};


    char storage[256 * tuple_size];
    container::pmr::monotonic_buffer_resource res{storage, sizeof(storage),
                                                  this_thread::get_default_resource()};
    container::pmr::polymorphic_allocator<void> alloc{&res};

    join_shared_state wss;

    template<typename T>
    using result_part = system::result<co_await_result_t<T>, std::exception_ptr>;

    std::tuple<result_part<Args> ...> result
        {
          result_part<Args>{system::in_place_error, std::exception_ptr()}...
        };
    std::exception_ptr error;
    constexpr static std::array<bool, sizeof...(Args)> interruptible{
        (std::is_lvalue_reference_v<Args> || is_interruptible_v<Args>)...};

    void cancel_all()
    {
      for (auto i = 0u; i < tuple_size; i++)
      {
        auto &r = cancel[i];
        if (interruptible[i] && r)
          r->emit(interrupt_await);
        if (r)
          std::exchange(r, nullptr)->emit(asio::cancellation_type::all);
      }
    }

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};

    template<std::size_t Idx, typename Aw>
    void await_suspend_step(
        executor exec, Aw && aw)
    {
      if (error && interruptible[Idx])
        return;
      if (!ready[Idx])
      {
        suspend_for_callback(
          aw,
          bind_completion_handler(
            (cancel[Idx] = &cancel_[Idx])->slot(),
            exec,
            alloc,
            [this, &aw, c = wss.get_completer()]() mutable
            {
              this->cancel[Idx] = nullptr;
              auto &re_ = std::get<Idx>(result) = get_resume_result(aw);
              if (re_.has_error() && error == nullptr)
              {
                error = re_.error();
                cancel_all();
              }
              c();
            }
          )
        );

        if (error && cancel[Idx])
          cancel[Idx]->emit(asio::cancellation_type::all);
      }
      else
      {
        auto & re_ = std::get<Idx>(result) = get_resume_result(aw);
        if (re_.has_error() && error == nullptr)
        {
          error = re_.error();
          cancel_all();
        }
      }
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>
          ([&](auto idx)
          {
            await_suspend_step<idx>(get_executor(h), std::get<idx>(aws));
          });

      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      assign_cancellation(
              h,
              [&](asio::cancellation_type ct)
              {
                for (auto & cs : cancel)
                  if (cs)
                    cs->emit(ct);
              });

      wss.h.reset(h.address());
      return true;
    }

    template<typename T>
    static T                   make_result_step(system::result<T, std::exception_ptr>    & rr) { return *std::move(rr);}
    static variant2::monostate make_result_step(system::result<void, std::exception_ptr> & )   { return {}; }


    template<typename ... Ts>
    static auto make_result(system::result<Ts, std::exception_ptr> & ... rr)
    {
      constexpr auto all_void = (std::is_void_v<Ts> && ...);
      if constexpr (!all_void)
        return std::make_tuple(make_result_step(rr)...);
    }

    auto await_resume()
    {
      wss.h.release();
      if (error)
        std::rethrow_exception(error);
      return std::apply([](auto & ... args) { return make_result(args...); }, result);
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable(args, std::make_index_sequence<sizeof...(Args)>{});
  }
};

template<typename Range>
struct join_ranged_impl
{
  Range aws;

  using result_type = co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<Range>()))>>;

  constexpr static std::size_t result_size =
      sizeof(std::conditional_t<std::is_void_v<result_type>, variant2::monostate, result_type>);

  struct awaitable
  {
    using type = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;
    container::pmr::monotonic_buffer_resource res;
    container::pmr::polymorphic_allocator<void> alloc{&res};

    std::conditional_t<awaitable_type<type>, Range &,
                       container::pmr::vector<co_awaitable_type<type>>> aws;

    container::pmr::vector<bool> ready{std::size(aws), alloc};
    container::pmr::vector<asio::cancellation_signal> cancel_{std::size(aws), alloc};
    container::pmr::vector<asio::cancellation_signal*> cancel{std::size(aws), alloc};
    container::pmr::vector<system::result<result_type, std::exception_ptr>> result{
          cancel.size(),
          system::result<result_type, std::exception_ptr>{system::in_place_error, std::exception_ptr()},
          alloc};

    constexpr static bool interruptible =
        std::is_lvalue_reference_v<Range> || is_interruptible_v<type>;

    std::exception_ptr error;

    awaitable(Range & aws_, std::false_type /* needs  operator co_await */)
      : res((256 + sizeof(co_awaitable_type<type>) + result_size) * std::size(aws_),
            this_thread::get_default_resource())
      , aws{alloc}
      , ready{std::size(aws_), alloc}
      , cancel_{std::size(aws_), alloc}
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
        : res((256 + sizeof(co_awaitable_type<type>) + result_size) * std::size(aws),
              this_thread::get_default_resource())
        , aws(aws)
    {
      std::transform(std::begin(aws), std::end(aws), std::begin(ready), [](auto & aw) {return aw.await_ready();});
    }

    awaitable(Range & aws)
      : awaitable(aws, std::bool_constant<awaitable_type<type>>{})
    {
    }

    join_shared_state wss;

    void cancel_all()
    {
      for (auto & r : cancel)
      {
        if (r && interruptible)
          r->emit(interrupt_await);
        if (r)
          std::exchange(r, nullptr)->emit(asio::cancellation_type::all);
      }
    }

    bool await_ready(){return std::find(ready.begin(), ready.end(), false) == ready.end();};
    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      // default cb
      auto exec = get_executor(h);

      for(std::size_t idx = 0u; idx < std::size(aws); idx++)
      {
        auto & aw = aws[idx];
        if (error && std::is_lvalue_reference_v<Range>)
          break;

        if (!ready[idx])
        {
          suspend_for_callback(
              aw,
              bind_completion_handler(
                (cancel[idx] = &cancel_[idx])->slot(),
                exec, alloc,
                [this, c = wss.get_completer(), idx]() mutable
                {
                  this->cancel[idx] = nullptr;
                  auto & re_ = result[idx] = get_resume_result(aws[idx]);
                  if (re_.has_error() && error == nullptr)
                  {
                    error = re_.error();
                    cancel_all();
                  }
                  c();
                }
              )
            );
          if (error && cancel[idx])
            cancel[idx]->emit(asio::cancellation_type::all);
        }
        else
        {
          auto & re_ = result[idx] = get_resume_result(aws[idx]);
          if (re_.has_error() && error == nullptr)
          {
            error = re_.error();
            cancel_all();
          }
        }
      }
      if (wss.use_count == 0) // already done, no need to suspend
        return false;

      // arm the cancel
      assign_cancellation(
                h,
                [&](asio::cancellation_type ct)
                {
                  for (auto & cs : cancel)
                    if (cs)
                      cs->emit(ct);
                });

      wss.h.reset(h.address());
      return true;
    }

    asio::cancellation_slot sl;

    auto await_resume()
    {
      sl.clear();
      ignore_unused(wss.h.release());
      if (error)
        std::rethrow_exception(error);
      if constexpr (!std::is_void_v<result_type>)
      {
        container::pmr::vector<result_type> rr{this_thread::get_allocator()};
        rr.reserve(result.size());
        for (auto & t : result)
          rr.push_back(std::move(t).value());
        return rr;
      }
    }
  };
  awaitable operator co_await() && {return awaitable{aws};}
};

}

namespace boost::async
{

template<typename ... Args>
struct is_interruptible<detail::join_variadic_impl<Args...>> : std::true_type {};
template<typename Range>
struct is_interruptible<detail::join_ranged_impl<Range>> : std::true_type {};

}

#endif //BOOST_ASYNC_DETAIL_JOIN_HPP
