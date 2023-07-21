//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_SELECT_HPP
#define BOOST_ASYNC_DETAIL_SELECT_HPP

#include <boost/async/detail/await_result_helper.hpp>
#include <boost/async/detail/fork.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/async/detail/forward_cancellation.hpp>
#include <boost/async/this_thread.hpp>
#include <boost/async/detail/util.hpp>

#include <boost/asio/bind_allocator.hpp>
#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>


#include <boost/intrusive_ptr.hpp>
#include <boost/core/span.hpp>
#include <boost/variant2/variant.hpp>

#include <coroutine>
#include <optional>


namespace boost::async::detail
{

struct left_select_tag {};

template<asio::cancellation_type Ct, typename URBG, typename ... Args>
struct select_variadic_impl
{
  using tuple_type = std::tuple<decltype(get_awaitable_type(std::declval<Args>()))...>;

  template<typename URBG_>
  select_variadic_impl(URBG_ && g, Args && ... args)
      : args{std::forward<Args>(args)...}, g(std::forward<URBG_>(g))
  {
  }

  std::tuple<Args...> args;
  URBG g;

  constexpr static std::size_t tuple_size = sizeof...(Args);

  struct awaitable : fork::static_shared_state<256 * tuple_size>
  {
    template<std::size_t ... Idx>
    awaitable(std::tuple<Args...> & args, URBG & g, std::index_sequence<Idx...>) :
        aws{awaitable_type_getter<Args>(std::get<Idx>(args))...}
    {
      if constexpr (!std::is_same_v<URBG, left_select_tag>)
        std::shuffle(impls.begin(), impls.end(), g);

    }

    tuple_type aws;
    std::array<asio::cancellation_signal, tuple_size> cancel_;

    template<typename > constexpr static auto make_null() {return nullptr;};
    std::array<asio::cancellation_signal*, tuple_size> cancel = {make_null<Args>()...};

    std::size_t index{std::numeric_limits<std::size_t>::max()};
    pmr::polymorphic_allocator<void> alloc{&this->resource};

    constexpr static bool all_void = (std::is_void_v<co_await_result_t<Args>> && ... );
    std::optional<variant2::variant<void_as_monostate<co_await_result_t<Args>>...>> result;
    std::exception_ptr error;

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

    template<std::size_t Idx>
    static detail::fork await_impl(awaitable & this_)
    try
    {
      auto & aw = std::get<Idx>(this_.aws);

      auto transaction = [&this_, idx = Idx] {
        if (this_.has_result())
          boost::throw_exception(std::logic_error("Another transaction already started"));
        this_.cancel[idx] = nullptr;
        // reserve the index early bc
        this_.index = idx;
        this_.cancel_all();
      };

      co_await fork::set_transaction_function(transaction);
      // check manually if we're ready
      auto rd = aw.await_ready();
      if (!rd)
      {
        this_.cancel[Idx] = &this_.cancel_[Idx];
        co_await this_.cancel[Idx]->slot();
        // make sure the executor is set
        co_await detail::fork::wired_up;

        // do the await - this doesn't call await-ready again
        if constexpr (std::is_void_v<decltype(aw.await_resume())>)
        {
          co_await aw;
          if (!this_.has_result())
            this_.index = Idx;
          if constexpr(!all_void)
            if (this_.index == Idx)
              this_.result.emplace(variant2::in_place_index<Idx>);
        }
        else
        {
          auto val = co_await aw;
          if (!this_.has_result())
            this_.index = Idx;
          if (this_.index == Idx)
            this_.result.emplace(variant2::in_place_index<Idx>,  std::move(val));
        }
        this_.cancel[Idx] = nullptr;
      }
      else
      {
        if (!this_.has_result())
          this_.index = Idx;
        if constexpr (std::is_void_v<decltype(aw.await_resume())>)
        {
          aw.await_resume();
          if (this_.index == Idx)
            this_.result.emplace(variant2::in_place_index<Idx>);
        }
        else
        {
          if (this_.index == Idx)
            this_.result.emplace(variant2::in_place_index<Idx>, aw.await_resume());
          else
            aw.await_resume();
        }
        this_.cancel[Idx] = nullptr;
      }
      this_.cancel_all();
    }
    catch(...)
    {
      if (!this_.has_result())
        this_.index = Idx;
      if (this_.index == Idx)
        this_.error = std::current_exception();
    }

    std::array<detail::fork(*)(awaitable&), tuple_size> impls {
        []<std::size_t ... Idx>(std::index_sequence<Idx...>)
        {
          return std::array<detail::fork(*)(awaitable&), tuple_size>{&await_impl<Idx>...};
        }(std::make_index_sequence<tuple_size>{})
    };

    detail::fork last_forked;

    bool await_ready()
    {
      last_forked = impls[0](*this);
      return last_forked.done();
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      this->exec = &get_executor(h);
      last_forked.release().resume();

      if (!this->outstanding_work()) // already done, resume rightaway.
        return false;

      for (std::size_t idx = 1u;
           idx < tuple_size; idx++) // we'
      {
        auto l = impls[idx](*this);
        const auto d = l.done();
        l.release();
        if (d)
          break;
      }

      if (!this->outstanding_work()) // already done, resume rightaway.
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

      this->coro.reset(h.address());
      return true;
    }

    auto await_resume()
    {
      if constexpr (all_void)
        return index;
      else
        return std::move(*result);
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable{args, g, std::make_index_sequence<tuple_size>{}};
  }
};


template<asio::cancellation_type Ct, typename URBG, typename Range>
struct select_ranged_impl
{
  using result_type = co_await_result_t<std::decay_t<decltype(*std::begin(std::declval<Range>()))>>;
  template<typename URBG_>
  select_ranged_impl(URBG_ && g, Range && rng)
      : range{std::forward<Range>(rng)}, g(std::forward<URBG_>(g))
  {
  }

  Range range;
  URBG g;

  struct awaitable : fork::shared_state
  {
    using type = std::decay_t<decltype(*std::begin(std::declval<Range>()))>;
    std::size_t index{std::numeric_limits<std::size_t>::max()};

    std::conditional_t<
        std::is_void_v<result_type>,
        variant2::monostate,
        std::optional<result_type>> result;

    std::exception_ptr error;

    pmr::monotonic_buffer_resource res;
    pmr::polymorphic_allocator<void> alloc{&resource};

    std::conditional_t<awaitable_type<type>, Range &,
        pmr::vector<co_awaitable_type<type>>> aws;

    /* all below `reorder` is reordered
     *
     * cancel[idx] is for aws[reorder[idx]]
    */
    pmr::vector<std::size_t> reorder{std::size(aws), alloc};
    pmr::vector<asio::cancellation_signal> cancel_{std::size(aws), alloc};
    pmr::vector<asio::cancellation_signal*> cancel{std::size(aws), alloc};

    bool has_result() const {return index != std::numeric_limits<std::size_t>::max(); }

    awaitable(Range & aws_,
              URBG & g,
              std::false_type /* needs co_await */)
        : fork::shared_state((256 + sizeof(co_awaitable_type<type>) + sizeof(std::size_t)) * std::size(aws_))
        , aws{alloc}
        , reorder{std::size(aws_), alloc}
        , cancel_{std::size(aws_), alloc}
        , cancel{std::size(aws_), alloc}
    {
      aws.reserve(std::size(aws_));
      for (auto && a : aws_)
        aws.emplace_back(awaitable_type_getter<decltype(a)>(std::forward<decltype(a)>(a)));

      std::generate(reorder.begin(), reorder.end(), [i = std::size_t(0u)]() mutable {return i++;});
      if constexpr (!std::is_same_v<URBG, left_select_tag>)
        std::shuffle(reorder.begin(), reorder.end(), g);
    }

    awaitable(Range & aws, URBG & g, std::true_type /* needs co_await */)
        : fork::shared_state((256 + sizeof(co_awaitable_type<type>) + sizeof(std::size_t)) * std::size(aws))
        , aws(aws)
    {
      std::generate(reorder.begin(), reorder.end(), [i = std::size_t(0u)]() mutable {return i++;});
      if constexpr (!std::is_same_v<URBG, left_select_tag>)
        std::shuffle(reorder.begin(), reorder.end(), g);
    }

    awaitable(Range & aws, URBG & g)
        : awaitable(aws, g, std::bool_constant<awaitable_type<type>>{})
    {
    }

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

    static detail::fork await_impl(awaitable & this_, std::size_t idx)
    try
    {
      auto & aw = *std::next(std::begin(this_.aws), idx);
      auto transaction = [&this_, idx = idx] {
        if (this_.has_result())
          boost::throw_exception(std::logic_error("Another transaction already started"));
        this_.cancel[idx] = nullptr;
        // reserve the index early bc
        this_.index = idx;
        this_.cancel_all();
      };

      co_await fork::set_transaction_function(transaction);
      // check manually if we're ready
      auto rd = aw.await_ready();
      if (!rd)
      {
        this_.cancel[idx] = &this_.cancel_[idx];
        co_await this_.cancel[idx]->slot();
        // make sure the executor is set
        co_await detail::fork::wired_up;

        // do the await - this doesn't call await-ready again
        if constexpr (std::is_void_v<result_type>)
        {
          co_await aw;
          if (!this_.has_result())
            this_.index = idx;
        }
        else
        {
          auto val = co_await aw;
          if (!this_.has_result())
            this_.index = idx;
          if (this_.index == idx)
            this_.result.emplace(std::move(val));
        }
        this_.cancel[idx] = nullptr;
      }
      else
      {
        if (!this_.has_result())
          this_.index = idx;
        if constexpr (std::is_void_v<decltype(aw.await_resume())>)
          aw.await_resume();
        else
        {
          if (this_.index == idx)
            this_.result.emplace(aw.await_resume());
          else
            aw.await_resume();
        }
        this_.cancel[idx] = nullptr;
      }
      this_.cancel_all();
    }
    catch(...)
    {
      if (!this_.has_result())
        this_.index = idx;
      if (this_.index == idx)
        this_.error = std::current_exception();
    }

    detail::fork last_forked;

    bool await_ready()
    {
      last_forked = await_impl(*this, reorder.front());
      return last_forked.done();
    }

    template<typename H>
    auto await_suspend(std::coroutine_handle<H> h)
    {
      this->exec = &detail::get_executor(h);
      last_forked.release().resume();

      if (!this->outstanding_work()) // already done, resume rightaway.
        return false;

      for (auto itr = std::next(reorder.begin());
           itr < reorder.end(); std::advance(itr, 1)) // we'
      {
        auto l = await_impl(*this, *itr);
        auto d = l.done();
        l.release();
        if (d)
          break;
      }

      if (!this->outstanding_work()) // already done, resume rightaway.
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

      this->coro.reset(h.address());
      return true;
    }

    auto await_resume()
    {
      if (error)
        std::rethrow_exception(error);
      if constexpr (std::is_void_v<result_type>)
        return index;
      else
        return std::make_pair(index, *result);
    }
  };
  awaitable operator co_await() &&
  {
    return awaitable{range, g};
  }
};

}

#endif //BOOST_ASYNC_DETAIL_SELECT_HPP
