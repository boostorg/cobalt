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
#include <boost/async/detail/util.hpp>
#include <boost/async/this_thread.hpp>

#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/system/result.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>
#include "forward_cancellation.hpp"
namespace boost::async::detail
{

template<asio::cancellation_type Ct, typename ... Args>
struct wait_impl
{
  using result_type = std::tuple<system::result<co_await_result_t<Args>, std::exception_ptr>...>;

  template<std::size_t ... Idx>
  auto get_awaitables_(std::index_sequence<Idx...>)
  {
    return std::make_tuple(detail::get_awaitable_type(std::get<Idx>(args))...);
  }

  wait_impl(Args && ... p) : args(std::forward<Args>(p)...)
  {
  }

  struct awaitable_type;

  template<std::size_t Idx>
  struct step
  {
    struct promise_type
    {
      wait_impl::awaitable_type & ref;
      using cancellation_slot_type = asio::cancellation_slot;

      asio::cancellation_signal cancel;
      cancellation_slot_type get_cancellation_slot()
      {
        return cancel.slot();
      }

      void* operator new(std::size_t n, wait_impl::awaitable_type & ref, std::size_t )
      {
        return ref.memory_resource.allocate(n);
      }

      void operator delete(void *, std::size_t) {}

      promise_type(wait_impl::awaitable_type & ref, std::size_t idx) : ref(ref)
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

      step get_return_object() {return {};}


    };
    constexpr static std::size_t needed_size =
        64u + // experimented size (64 on gcc, 24 on clang)
        sizeof(promise_type) +
        sizeof(co_awaitable_type<std::tuple_element_t<Idx, std::tuple<Args...>>>) +
        sizeof(std::size_t)
    ;
  };

  awaitable_type operator co_await() &&
  {
    return std::make_from_tuple<awaitable_type>(std::move(args));
  }

 private:
  std::tuple<Args...> args;
};


template<asio::cancellation_type Ct, typename ... Args>
struct wait_impl<Ct, Args...>::awaitable_type
{
  template<std::size_t Idx>
  friend struct step;

  awaitable_type(Args && ... args) : awaitables(detail::get_awaitable_type(std::forward<Args>(args))...)
  {
    std::fill(cancel.begin(), cancel.end(), nullptr);
  }

  template<std::size_t Idx>
  bool await_ready_impl_step()
  {
    if (std::get<Idx>(awaitables).await_ready())
    {
      try
      {
        if constexpr (std::is_void_v<decltype(std::get<Idx>(awaitables).await_resume())>)
        {
          std::get<Idx>(awaitables).await_resume();
          std::get<Idx>(this->result) = system::result<void, std::exception_ptr>{};
        }
        else
          std::get<Idx>(this->result) = std::get<Idx>(awaitables).await_resume();
      }
      catch(std::exception & e)
      {
        std::get<Idx>(this->result) = std::tuple_element_t<Idx, result_type>(std::current_exception());
      }

      outstanding--;
      return true;
    }
    return false;
  }

  template<std::size_t ... Idx>
  bool await_ready_impl(std::index_sequence<Idx...>)
  {
    return (await_ready_impl_step<Idx>() && ... );
  };

  bool await_ready()
  {
    return await_ready_impl(std::make_index_sequence<sizeof...(Args)>{});
  }

  template<std::size_t Idx>
  step<Idx> await_suspend_impl_step(std::size_t = Idx)
  try
  {
    if constexpr (std::is_same_v<typename std::tuple_element_t<Idx, result_type>::value_type, void>)
    {
      co_await std::move(std::get<Idx>(awaitables));
      co_return variant2::monostate{};
    }
    else
      co_return co_await std::move(std::get<Idx>(awaitables));
  }
  catch (...)
  {
    throw;
  }

  template<std::size_t ... Idx>
  void await_suspend_impl(std::index_sequence<Idx...>)
  {
    (await_suspend_impl_step<Idx>(), ...);
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
        std::unique_ptr<void, detail::coro_deleter<void>> &awaited_from;
        do_cancel(std::array<asio::cancellation_signal*, sizeof...(Args)> &cancel,
                  std::unique_ptr<void, detail::coro_deleter<void>> &awaited_from)
            : cancel(cancel), awaited_from(awaited_from) {}
        void operator()(asio::cancellation_type ct)
        {
          if (ct == interrupt_await && awaited_from)
            return std::coroutine_handle<void>::from_address(awaited_from.release()).resume();

          for (auto & cs : cancel)
            if (cs) cs->emit(ct);
        }
      };

      sl.template emplace<do_cancel>(cancel, awaited_from);
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
  std::tuple<detail::co_awaitable_type<Args>...> awaitables;

  std::size_t outstanding{sizeof...(Args)};
  std::array<asio::cancellation_signal*, sizeof...(Args)> cancel;
  asio::cancellation_slot cancellation_slot;
  std::unique_ptr<void, detail::coro_deleter<void>> awaited_from;

  template<std::size_t ...Idx>
  constexpr static std::size_t step_size(std::index_sequence<Idx...>)
  {
    return (step<Idx>::needed_size + ...);
  }


  char memory_buffer[step_size(std::make_index_sequence<sizeof...(Args)>{})];
  container::pmr::monotonic_buffer_resource memory_resource{memory_buffer, sizeof(memory_buffer)};
  result_type result{wait_not_ready<Args>()...};
};

template<asio::cancellation_type Ct, typename PromiseRange>
struct ranged_wait_impl
{
  using awaitable_t =
      co_awaitable_type<std::decay_t<decltype(*std::begin(std::declval<PromiseRange>()))>>;

  using result_type = system::result<
      co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<PromiseRange>()))>>,
      std::exception_ptr>;

  using result_vector = std::vector<
          result_type,
          typename std::allocator_traits<
              asio::associated_allocator_t<std::decay_t<PromiseRange>>>::template rebind_alloc<result_type>
          >;

  ranged_wait_impl(PromiseRange && p) : range(static_cast<PromiseRange&&>(p))
  {
  }

  struct awaitable_type;
  struct step
  {
    struct promise_type
    {
      ranged_wait_impl::awaitable_type& ref;
      std::size_t idx;
      using cancellation_slot_type = asio::cancellation_slot;

      asio::cancellation_signal cancel;
      cancellation_slot_type get_cancellation_slot()
      {
        return cancel.slot();
      }

      void* operator new(std::size_t n, ranged_wait_impl::awaitable_type & ref, std::size_t )
      {
        return ref.memory_resource.allocate(n);
      }

      void operator delete(void *, std::size_t) {}

      promise_type(ranged_wait_impl::awaitable_type & ref, std::size_t idx) : ref(ref), idx(idx)
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

      step get_return_object() {return {};}
    };
    constexpr static std::size_t needed_size =
        64u + // experimented size (64 on gcc, 24 on clang)
        sizeof(promise_type) +
        sizeof(co_awaitable_type<awaitable_t>) +
        sizeof(std::size_t);
  };
  struct awaitable_type
  {

    friend struct step;

    awaitable_type(awaitable_type &&) = delete;

    template<typename Range>
    awaitable_type(Range &&range)
        : outstanding{std::size(range)}
        , memory_resource{
          ((step::needed_size + sizeof(awaitable_t) + sizeof(asio::cancellation_signal *)) * std::size(range)),
            this_thread::get_default_resource()}
        , cancel{std::size(range), nullptr, alloc}
        , result(asio::get_associated_allocator(range))
    {
      awaitables.reserve(std::size(range));
      if constexpr (std::is_rvalue_reference_v<PromiseRange&&>)
        for (auto && v : range)
          awaitables.push_back(get_awaitable_type(std::move(v)));
      else
        for (auto && v : range)
          awaitables.push_back(get_awaitable_type(v));

      result.resize(std::size(range), result_type(detail::wait_not_ready()));

      was_ready.resize(std::size(range), false);
      std::fill(cancel.begin(), cancel.end(), nullptr);
    }

    bool await_ready()
    {
      if (std::empty(awaitables))
        return true;

      bool all_ready = true;

      std::size_t idx = 0u;
      for (auto &r: awaitables)
      {
        if (r.await_ready())
        {
          try
          {
            if constexpr (std::is_void_v<typename result_type::value_type>)
            {
              r.await_resume();
              this->result[idx] = system::result<void, std::exception_ptr>{};
            } else
              this->result[idx] = r.await_resume();
          }
          catch (std::exception &e)
          {
            this->result[idx] = result_type(std::current_exception());
          }
          was_ready[idx] = true;
          this->outstanding--;
        } else
          all_ready = false;
        idx++;
      }
      return all_ready;
    };


    step await_suspend_impl(std::size_t idx)
    try
    {
      if constexpr (std::is_same_v<void, typename result_type::value_type>)
      {
        co_await std::move(awaitables[idx]);
        co_return variant2::monostate{};
      } else
        co_return co_await std::move(awaitables[idx]);
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
          container::pmr::vector<asio::cancellation_signal *> &cancel;
          std::unique_ptr<void, detail::coro_deleter<void>> &awaited_from;

          do_cancel(container::pmr::vector<asio::cancellation_signal *> &cancel,
                    std::unique_ptr<void, detail::coro_deleter<void>> &awaited_from)
              : cancel(cancel), awaited_from(awaited_from)
          {}

          void operator()(asio::cancellation_type ct)
          {
            if (ct == interrupt_await && awaited_from)
              return std::coroutine_handle<void>::from_address(awaited_from.release()).resume();
            for (auto &cs: cancel)
              if (cs) cs->emit(ct);
          }
        };

        sl.template emplace<do_cancel>(cancel, awaited_from);
      }
      awaited_from.reset(h.address());
      for (std::size_t i = 0u; i < std::size(awaitables); i++)
        if (!was_ready[i])
          await_suspend_impl(i);
    }

    result_vector await_resume()
    {
      if (cancellation_slot.is_connected())
        cancellation_slot.clear();

      if constexpr (!std::is_void_v<result_type>)
        return std::move(result);
    }
   private:
    std::size_t outstanding;
    container::pmr::monotonic_buffer_resource memory_resource{
        ((step::needed_size + sizeof(awaitable_t) + sizeof(asio::cancellation_signal*)) * std::size(awaitables)),
        this_thread::get_default_resource()};

    container::pmr::polymorphic_allocator<void> alloc{&memory_resource};
    container::pmr::vector<awaitable_t> awaitables{alloc};
    container::pmr::vector<asio::cancellation_signal*> cancel{alloc};

    asio::cancellation_slot cancellation_slot;
    std::unique_ptr<void, detail::coro_deleter<void>> awaited_from;
    container::pmr::vector<bool> was_ready{cancel.get_allocator()};
    result_vector result{};
  };

  awaitable_type operator co_await() &&
  {
    return awaitable_type(std::forward<PromiseRange>(range));
  }

 private:
  PromiseRange range;

};


}

#endif //BOOST_ASYNC_DETAIL_WAIT_HPP
