//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_RACEa_HPP
#define BOOST_ASYNC_DETAIL_RACEa_HPP

#include <boost/async/detail/await_result_helper.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/this_thread.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/core/span.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>
#include <optional>


namespace boost::async::detail
{


struct race_shared_state
{
  std::unique_ptr<void, coro_deleter<>> h;
  std::size_t use_count = 0u;

  friend void intrusive_ptr_add_ref(race_shared_state * st) {st->use_count++;}
  friend void intrusive_ptr_release(race_shared_state * st) {if (st->use_count-- == 1u) st->h.reset();}

  void complete()
  {
    if (use_count == 0u && h != nullptr)
      std::coroutine_handle<void>::from_address(h.release()).resume();
  }

  struct completer
  {
    intrusive_ptr<race_shared_state> ptr;
    completer(race_shared_state * wss) : ptr{wss} {}

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

template<asio::cancellation_type Ct, typename ... Args>
struct race_variadic_impl
{
  using tuple_type = std::tuple<decltype(get_awaitable_type(std::declval<Args>()))...>;

  race_variadic_impl(Args && ... args)
      : args{std::forward<Args>(args)...}
  {
  }

  std::tuple<Args...> args;

  constexpr static std::size_t tuple_size = sizeof...(Args);

  struct awaitable
  {
    template<std::size_t ... Idx>
    awaitable(std::tuple<Args...> & args, std::index_sequence<Idx...>) :
        aws{awaitable_type_getter<Args>(std::get<Idx>(args))...}
    {
    }

    tuple_type aws;

    std::array<bool, tuple_size> ready{};
    std::array<asio::cancellation_signal, tuple_size> cancel_;

    template<typename > constexpr static auto make_null() {return nullptr;};
    std::array<asio::cancellation_signal*, tuple_size> cancel = {make_null<Args>()...};

    std::size_t index{std::numeric_limits<std::size_t>::max()};
    std::size_t spawned = 0u;
    char storage[256 * tuple_size];
    container::pmr::monotonic_buffer_resource res{storage, sizeof(storage),
                                                  this_thread::get_default_resource()};
    container::pmr::polymorphic_allocator<void> alloc{&res};

    race_shared_state sss;

    bool has_result() const
    {
      return index != std::numeric_limits<std::size_t>::max();
    }

    void cancel_all()
    {
      interrupt_await();
      for (auto i = 0u; i < tuple_size; i++)
        if (auto &r = cancel[i]; r)
          std::exchange(r, nullptr)->emit(Ct);
    }


    template<std::size_t Idx>
    void interrupt_await_step()
    {
      using type= std::tuple_element_t<Idx, tuple_type>;
      using t = std::conditional_t<std::is_reference_v<std::tuple_element_t<Idx, std::tuple<Args...>>>,
          type &,
          type &&>;

      if constexpr (interruptible<t>)
        static_cast<t>(std::get<Idx>(aws)).interrupt_await();
    }

    void interrupt_await()
    {
      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>
          ([&](auto idx)
           {
             interrupt_await_step<idx>();
           });
    }

    bool await_ready()
    {
      bool found_ready = false;
      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>(
          [&](auto idx)
          {
            if (!found_ready || !interruptible<std::tuple_element_t<idx, tuple_type>>)
              found_ready |= ready[idx] = std::get<idx>(aws).await_ready();
            else
              ready[idx] = false;
          });

      return found_ready;
    }

    template<typename Aw>
    void await_suspend_step(
        executor exec,
        Aw && aw, std::size_t idx)
    {
      if (has_result() && interruptible<Aw&&>)
        return ; // one coro did a direct complete
      else if (!has_result())
        spawned = idx;

      if (!ready[idx])
      {
        suspend_for_callback_with_transaction(
          aw,
          [this, idx]
          {
            if (has_result())
              boost::throw_exception(std::logic_error("Another transaction already started"),
                                     BOOST_CURRENT_LOCATION);
            this->cancel[idx] = nullptr;
            this->index = idx;
            this->cancel_all();
          },
          bind_completion_handler(
              (cancel[idx] = &cancel_[idx])->slot(),
              exec,
              alloc,
              [this, idx, c=sss.get_completer()]() mutable
              {
                this->cancel[idx] = nullptr;
                if (!has_result())
                  index = idx;
                this->cancel_all();
                c();
              }
            )
          );
        if (has_result())
          if (this->cancel[idx])
            this->cancel[idx]->emit(Ct);
      }
      else if (!has_result())
      {
        index = idx;
        this->cancel_all();
      }

    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      auto exec = get_executor(h);
      std::size_t idx = 0u;
      mp11::tuple_for_each(
          aws,
          [&](auto && aw)
          {
            await_suspend_step(exec, aw, idx++);
          });
      if (sss.use_count == 0) // already done, no need to suspend
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

      if (!has_result())
      {
        sss.h.reset(h.address());
        return true;
      }
      else // short circuit here, great.
        return false;
    }


    template<typename T>
    using void_as_monostate = std::conditional_t<std::is_void_v<T>, variant2::monostate, T>;
    constexpr static bool all_void = (std::is_void_v<co_await_result_t<Args>> && ... );
    using result_type = std::conditional_t<
              all_void,
              std::size_t,
              variant2::variant<void_as_monostate<co_await_result_t<Args>>...>>;

    result_type await_resume()
    {
      if (!has_result())
        index = std::distance(ready.begin(), std::find(ready.begin(), ready.end(), true));
      BOOST_ASSERT(has_result());

      mp11::mp_for_each<mp11::mp_iota_c<sizeof...(Args)>>(
          [&](auto iidx)
          {
            constexpr std::size_t idx = iidx;
            try
            {
              using type= std::tuple_element_t<iidx, tuple_type>;
              using t = std::conditional_t<std::is_reference_v<std::tuple_element_t<iidx, std::tuple<Args...>>>,
                  type &,
                  type &>;

              if ((index != idx) &&
                  ((idx <= spawned) || !interruptible<t> || ready[idx]))
                std::get<idx>(aws).await_resume();
            }
            catch (...) {}
          });


      return mp11::mp_with_index<sizeof...(Args)>(
          index,
          [this](auto idx) -> result_type
          {
              constexpr std::size_t sz = idx;

              if constexpr (all_void)
              {
                std::get<sz>(aws).await_resume();
                return sz;
              }
              else if constexpr (std::is_void_v<decltype(std::get<sz>(aws).await_resume())>)
              {
                std::get<sz>(aws).await_resume();
                return result_type(variant2::in_place_index<sz>);
              }
              else
                return result_type(variant2::in_place_index<sz>,
                                   std::get<sz>(aws).await_resume());
          });
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable{args, std::make_index_sequence<tuple_size>{}};
  }
};


template<asio::cancellation_type Ct, typename Range>
struct race_ranged_impl
{
  using result_type = co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<Range>()))>>;
  race_ranged_impl(Range && rng)
      : range{std::forward<Range>(rng)}
  {
  }

  Range range;

  struct awaitable
  {
    using type = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;
    std::size_t index{std::numeric_limits<std::size_t>::max()};
    std::size_t spawned = 0u;

    container::pmr::monotonic_buffer_resource res;
    container::pmr::polymorphic_allocator<void> alloc{&res};

    std::conditional_t<awaitable_type<type>, Range &,
        container::pmr::vector<co_awaitable_type<type>>> aws;

    container::pmr::vector<bool> ready{std::size(aws), alloc};
    container::pmr::vector<asio::cancellation_signal> cancel_{std::size(aws), alloc};
    container::pmr::vector<asio::cancellation_signal*> cancel{std::size(aws), alloc};

    bool has_result() const {return index != std::numeric_limits<std::size_t>::max(); }

    awaitable(Range & aws_, std::false_type /* needs co_await */)
        : res((256 + sizeof(co_awaitable_type<type>)) * std::size(aws_),
              this_thread::get_default_resource())
        , aws{alloc}
        , ready{std::size(aws_), alloc}
        , cancel_{std::size(aws_), alloc}
        , cancel{std::size(aws_), alloc}
    {
      aws.reserve(std::size(aws_));
      for (auto && a : aws_)
        aws.emplace_back(awaitable_type_getter<decltype(a)>(std::forward<decltype(a)>(a)));
    }

    awaitable(Range & aws, std::true_type /* needs co_await */)
        : res((256 + sizeof(co_awaitable_type<type>)) * std::size(aws),
              this_thread::get_default_resource())
        , aws(aws)
    {
    }

    awaitable(Range & aws)
        : awaitable(aws, std::bool_constant<awaitable_type<type>>{})
    {
    }

    race_shared_state sss;

    void cancel_all()
    {
      interrupt_await();
      for (auto & r : cancel)
        if (r)
          std::exchange(r, nullptr)->emit(Ct);
    }
    void interrupt_await()
    {
      using t = std::conditional_t<std::is_reference_v<Range>,
          co_awaitable_type<type> &,
          co_awaitable_type<type> &&>;
      if constexpr (interruptible<t>)
        for (auto & aw : aws)
          static_cast<t>(aw).interrupt_await();
    }

    bool await_ready()
    {
      bool found_ready = false;
      std::transform(
          std::begin(aws), std::end(aws), std::begin(ready),
          [&found_ready](auto & aw)
          {
            using t = std::conditional_t<std::is_reference_v<Range>,
                co_awaitable_type<type> &,
                co_awaitable_type<type> &&>;

            if (!found_ready || !interruptible<t>)
            {
              auto r = aw.await_ready();
              found_ready |= r;
              return r;
            }
            else
              return false;
          });

      return found_ready;
    }

    template<typename Aw>
    void await_suspend_step(
        executor exec,
        Aw && aw, std::size_t idx)
    {
      this->cancel[idx] = &this->cancel_[idx];
      if (!ready[idx])
      {
        suspend_for_callback_with_transaction(
          aw,
          [this, idx]
          {
            if (has_result())
              boost::throw_exception(std::logic_error("Another transaction already started"),
                                     BOOST_CURRENT_LOCATION);
            this->cancel[idx] = nullptr;
            this->index = idx;
            this->cancel_all();
          },
          bind_completion_handler(
            cancel[idx]->slot(),
            exec,
            alloc,
            [this, idx, c=sss.get_completer()]() mutable
            {
              this->cancel[idx] = nullptr;
              if (!has_result())
                index = idx;
              this->cancel_all();
              c();
            })
          );
        if (has_result())
          if (this->cancel[idx])
            this->cancel[idx]->emit(Ct);
      }
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      std::size_t idx = 0u;
      for (auto && aw : aws)
      {
        if (has_result() && std::is_lvalue_reference_v<Range>)
          break; // one coro did a direct complete
        else if (!has_result())
          spawned = idx;

        await_suspend_step(get_executor(h), aw, idx++);
      }

      if (sss.use_count == 0) // already done, no need to suspend
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

      if (!has_result())
      {
        sss.h.reset(h.address());
        return true;
      }
      else // short circuit here, great.
        return false;

    }

    auto await_resume()
    {
      if (!has_result())
        index = std::distance(ready.begin(), std::find(ready.begin(), ready.end(), true));
      BOOST_ASSERT(index != ready.size());

      for (std::size_t idx = 0u; idx < aws.size(); idx++)
      {
        try
        {
          if ((index != idx)
              && ((idx <= spawned) || !std::is_lvalue_reference_v<Range> || ready[idx]))
            aws[idx].await_resume();
        }
        catch (...) {}
      }
      if constexpr (std::is_void_v<result_type>)
      {
        aws[index].await_resume();
        return index;
      }
      else
        return std::make_pair(index, aws[index].await_resume());
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable{range};
  }
};

}

#endif //BOOST_ASYNC_DETAIL_RACEa_HPP
