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
#include <boost/async/detail/util.hpp>
#include <boost/async/this_thread.hpp>

#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/system/result.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>

namespace boost::async::detail
{

template<asio::cancellation_type Ct, typename ... Args>
struct wait_impl
{
  using result_type = std::tuple<system::result<co_await_result_t<Args>, std::exception_ptr>...>;


  wait_impl(Args && ... p) : args(static_cast<Args&&>(p)...) {std::fill(cancel.begin(), cancel.end(), nullptr);}
  wait_impl(wait_impl && lhs) : args(std::move(lhs.args)) {std::fill(cancel.begin(), cancel.end(), nullptr);}

  template<std::size_t ... Idx>
  bool await_ready_impl(std::index_sequence<Idx...>)
  {
    return ((
        [this]
        {
          if constexpr (requires {std::get<Idx>(args).ready(); std::get<Idx>(args).get();})
            if (std::get<Idx>(args).ready())
            {
              try
              {
                if constexpr (std::is_void_v<decltype(std::get<Idx>(args).get())>)
                  std::get<Idx>(this->result) = system::result<void, std::exception_ptr>{};
                else
                  std::get<Idx>(this->result) = std::get<Idx>(args).get();
              }
              catch(...)
              {
                std::get<Idx>(this->result) = std::tuple_element_t<Idx, result_type>(std::current_exception());
              }

              outstanding--;
              return true;
            }
          return false;
        }()) && ... );
  };

  bool await_ready()
  {
    return await_ready_impl(std::make_index_sequence<sizeof...(Args)>{});
  }

  template<std::size_t Idx>
  struct stub
  {
    struct promise_type
    {
      wait_impl & ref;
      using cancellation_slot_type = asio::cancellation_slot;

      asio::cancellation_signal cancel;
      cancellation_slot_type get_cancellation_slot()
      {
        return cancel.slot();
      }

      void* operator new(std::size_t n, wait_impl & ref, std::size_t )
      {
        return ref.memory_resource.allocate(n);
      }

      void operator delete(void *, std::size_t) {}

      promise_type(wait_impl & ref, std::integral_constant<std::size_t, Idx> idx) : ref(ref)
      {
        ref.cancel[idx] = &cancel;
      }

      constexpr static std::suspend_never initial_suspend() noexcept { return {}; }
      auto final_suspend() noexcept
      {
        ref.cancel[Idx] = nullptr;

        struct final_awaitable
        {
          constexpr static bool await_ready() noexcept {return false;}

          std::coroutine_handle<void> await_suspend(
              std::coroutine_handle<promise_type> h) noexcept
          {
            auto & rf = h.promise().ref;
            std::coroutine_handle<void> res = std::noop_coroutine();

            h.destroy();
            if (--rf.outstanding == 0u)
              res = std::coroutine_handle<void>::from_address(rf.awaited_from.release());

            return res;
          }
          void await_resume() noexcept {}
        };
        return final_awaitable{};
      }

      void unhandled_exception()
      {
        std::get<Idx>(ref.result) = std::current_exception();
      }

      void return_value(variant2::monostate &&)
      {
        std::get<Idx>(ref.result) = system::result<void, std::exception_ptr>{};
        ref.cancel[Idx] = nullptr;
      }

      template<typename T>
      void return_value(T && t)
      {
        std::get<Idx>(ref.result) = std::forward<T>(t);
        ref.cancel[Idx] = nullptr;
      }

      stub get_return_object() {return {};}
    };

  };
  template<std::size_t Idx>
  stub<Idx> await_suspend_impl_step(std::integral_constant<std::size_t, Idx>)
  try
  {
    if constexpr (std::is_same_v<typename std::tuple_element_t<Idx, result_type>::value_type, void>)
    {
      co_await std::move(std::get<Idx>(args));
      co_return variant2::monostate{};
    }
    else
      co_return co_await std::move(std::get<Idx>(args));
  }
  catch (...)
  {
    throw;
  }

  template<std::size_t ... Idx>
  void await_suspend_impl(std::index_sequence<Idx...>)
  {
    (await_suspend_impl_step(std::integral_constant<std::size_t, Idx>{}), ...);
  }

  template<typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h)
  {

    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
    {
      struct do_cancel
      {
        std::array<asio::cancellation_signal*, sizeof...(Args)> &cancel;
        do_cancel(std::array<asio::cancellation_signal*, sizeof...(Args)> &cancel) : cancel(cancel) {}
        void operator()(asio::cancellation_type ct)
        {
          for (auto & cs : cancel)
            if (cs) cs->emit(ct);
        }
      };

      sl.template emplace<do_cancel>(cancel);
    }
    awaited_from.reset(h.address());
    await_suspend_impl(std::make_index_sequence<sizeof...(Args)>{});
  }

  auto await_resume()
  {
    if (cancellation_slot.is_connected())
      cancellation_slot.clear();

    return std::move(result);
  }

 private:
  std::tuple<Args...> args;

  std::size_t outstanding{sizeof...(Args)};
  std::array<asio::cancellation_signal*, sizeof...(Args)> cancel;
  asio::cancellation_slot cancellation_slot;
  std::unique_ptr<void, detail::coro_deleter<void>> awaited_from;

  result_type result{wait_not_ready<Args>()...};

  char memory_buffer[((1200 + sizeof(std::conditional_t<std::is_void_v<co_await_result_t<Args>>, variant2::monostate, co_await_result_t<Args>>)) + ...)];
  container::pmr::monotonic_buffer_resource memory_resource{memory_buffer, sizeof(memory_buffer)};
};


template<asio::cancellation_type Ct, typename PromiseRange>
struct ranged_wait_impl
{
  using result_type = system::result<
      co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<PromiseRange>()))>>,
      std::exception_ptr>;

  ranged_wait_impl(PromiseRange && p) : range(static_cast<PromiseRange&&>(p))
  {
    result.resize(std::size(range), result_type(detail::wait_not_ready()));
    was_ready.resize(std::size(range), false);
    std::fill(cancel.begin(), cancel.end(), nullptr);
  }
  ranged_wait_impl(ranged_wait_impl && lhs)
      : range(static_cast<PromiseRange&&>(lhs.range)), result(std::move(lhs.result)), was_ready(std::move(lhs.was_ready))
  {
    std::fill(cancel.begin(), cancel.end(), nullptr);
  }
  bool await_ready()
  {
    if (std::empty(range))
        return true;
    if constexpr (requires {std::begin(range)->ready(); std::begin(range)->get();})
    {
      bool all_ready = true;
      for (std::size_t idx = 0u; idx < std::size(range); idx++)
      {
        auto & r = range[idx];
        if (r.ready())
        {
          try
          {
            if constexpr (std::is_void_v<typename result_type::value_type>)
              this->result[idx] = system::result<void, std::exception_ptr>{};
            else
              this->result[idx] = r.get();
          }
          catch(...)
          {
            this->result[idx] = result_type(std::current_exception());
          }
          was_ready[idx] = true;
          this->outstanding--;
        }
        else
          all_ready = false;
      }
      return all_ready;
    }
    else
      return false;

  };

  struct stub
  {
    struct promise_type
    {
      ranged_wait_impl & ref;
      std::size_t idx;
      using cancellation_slot_type = asio::cancellation_slot;

      asio::cancellation_signal cancel;
      cancellation_slot_type get_cancellation_slot()
      {
        return cancel.slot();
      }

      void* operator new(std::size_t n, ranged_wait_impl & ref, std::size_t )
      {
        return ref.memory_resource.allocate(n);
      }

      void operator delete(void *, std::size_t) {}

      promise_type(ranged_wait_impl & ref, std::size_t idx) : ref(ref), idx(idx)
      {
        ref.cancel[idx] = &cancel;
      }

      constexpr static std::suspend_never initial_suspend() noexcept { return {}; }
      auto final_suspend() noexcept
      {
        ref.cancel[idx] = nullptr;

        struct final_awaitable
        {
          constexpr static bool await_ready() noexcept {return false;}

          std::coroutine_handle<void> await_suspend(
              std::coroutine_handle<promise_type> h) noexcept
          {
            auto & rf = h.promise().ref;
            std::coroutine_handle<void> res = std::noop_coroutine();

            h.destroy();
            if (--rf.outstanding == 0u)
              res = std::coroutine_handle<void>::from_address(rf.awaited_from.release());

            return res;
          }
          void await_resume() noexcept {}
        };
        return final_awaitable{};
      }

      void unhandled_exception()
      {
        ref.result[idx] = std::current_exception();
      }

      template<typename T>
      void return_value(T && t)
      {
        if constexpr (std::is_same_v<void, typename result_type::value_type>)
          ref.result[idx] = system::result<void, std::exception_ptr>();
        else
          ref.result[idx] = std::forward<T>(t);

        ref.cancel[idx] = nullptr;

      }

      stub get_return_object() {return {};}
    };

  };

  stub await_suspend_impl(std::size_t idx)
  try
  {
    if constexpr (std::is_same_v<void, typename result_type::value_type>)
    {
      co_await std::move(*std::next(range.begin(), idx));
      co_return variant2::monostate{};
    }
    else
      co_return co_await std::move(*std::next(range.begin(), idx));
  }
  catch (...)
  {
    throw;
  }

  template<typename Promise>
  void await_suspend(std::coroutine_handle<Promise> h)
  {
    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
    {
      struct do_cancel
      {
        container::pmr::vector<asio::cancellation_signal*> &cancel;
        do_cancel(container::pmr::vector<asio::cancellation_signal*> &cancel) : cancel(cancel) {}
        void operator()(asio::cancellation_type ct)
        {
          for (auto & cs : cancel)
            if (cs) cs->emit(ct);
        }
      };

      sl.template emplace<do_cancel>(cancel);
    }
    awaited_from.reset(h.address());
    for (std::size_t i = 0u; i < std::size(range); i++)
      if (!was_ready[i])
        await_suspend_impl(i);
  }

  container::pmr::vector<result_type> await_resume()
  {
    if (cancellation_slot.is_connected())
      cancellation_slot.clear();

    if constexpr (!std::is_void_v<result_type>)
      return std::move(result);
  }

 private:
  PromiseRange range;
  std::size_t outstanding{std::size(range)};

  container::pmr::monotonic_buffer_resource memory_resource{((1400 + sizeof(result_type)) * std::size(range)),
                                                            this_thread::get_default_resource()};
  container::pmr::vector<asio::cancellation_signal*> cancel{std::size(range),
                                                            nullptr,
                                                            container::pmr::polymorphic_allocator<asio::cancellation_signal*>(&memory_resource)};

  asio::cancellation_slot cancellation_slot;
  std::unique_ptr<void, detail::coro_deleter<void>> awaited_from;
  container::pmr::vector<bool> was_ready{cancel.get_allocator()};
  container::pmr::vector<result_type> result{cancel.get_allocator()};
};


}

#endif //BOOST_ASYNC_DETAIL_WAIT_HPP
