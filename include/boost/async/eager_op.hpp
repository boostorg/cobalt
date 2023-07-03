//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_EAGER_OP_HPP
#define BOOST_ASYNC_EAGER_OP_HPP

#include <boost/async/detail/handler.hpp>
#include <boost/async/detail/exception.hpp>

#include <boost/container/pmr/monotonic_buffer_resource.hpp>

namespace boost::async
{

template<typename ... Args>
struct eager_op
{
  virtual void initiate(async::completion_handler<Args...> complete) = 0 ;
  virtual ~eager_op() = default;

  struct awaitable
  {
    eager_op<Args...> &eager_op_;
    std::exception_ptr error;
    std::optional<std::tuple<Args...>> result;
    async::completion_handler<Args...> *completion;

    awaitable(eager_op<Args...> * eager_op_) : eager_op_(*eager_op_) {}
    awaitable(awaitable && lhs)
        : eager_op_(lhs.eager_op_)
        , error(std::move(lhs.error))
        , result(std::move(lhs.result))
        , completion(lhs.completion)
    {
      if (completion)
          completion->result = & result;
    }

    bool await_ready(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      if (!completion && !result.has_value())
        detail::throw_already_awaited(loc);
      return result.has_value();
    }
    bool completed_immediately = false;

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> h)
    {
      BOOST_ASSERT(completion);
      completion->self.reset(h.address());
      completion->cancellation_slot = asio::get_associated_cancellation_slot(h.promise());
    }

    auto await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      return interpret_result(*std::move(result), loc);
    }

    void launch()
    {
      eager_op_.initiate(
          completion_handler<Args...>(
            std::noop_coroutine(),
            result,
            this_thread::get_allocator().resource(),
            &completed_immediately,
            &completion));
    }

    void interrupt_await()
    {
      if (completion && !result)
      {
        completion->result = nullptr;
        completion->completed_immediately = nullptr;
        fill_result_(mp11::mp_list<Args...>{});
        std::coroutine_handle<void>::from_address(completion->self.release()).resume();
      }
    }
   private:

    void fill_result_(mp11::mp_list<>) {result.emplace();}
    template<typename ...Args_>
    void fill_result_(mp11::mp_list<system::error_code, Args_...>)
    {
      result.emplace(asio::error::interrupted, Args_()...);
    }
    template<typename ...Args_>
    void fill_result_(mp11::mp_list<std::exception_ptr, Args_...>)
    {
      result.emplace(detail::detached_exception(), Args_()...);
    }

  };

  awaitable operator co_await()
  {
    auto res =  awaitable{this};
    res.launch();
    return res;
  }
};

struct use_eager_op_t
{
  /// Default constructor.
  constexpr use_eager_op_t()
  {
  }

  /// Adapts an executor to add the @c deferred_t completion token as the
  /// default.
  template <typename InnerExecutor>
  struct executor_with_default : InnerExecutor
  {
    /// Specify @c deferred_t as the default completion token type.
    typedef use_eager_op_t default_completion_token_type;

    executor_with_default(const InnerExecutor& ex) BOOST_ASIO_NOEXCEPT
        : InnerExecutor(ex)
    {
    }

    /// Construct the adapted executor from the inner executor type.
    template <typename InnerExecutor1>
    executor_with_default(const InnerExecutor1& ex,
                          typename std::enable_if<
                              std::conditional<
                              !std::is_same<InnerExecutor1, executor_with_default>::value,
                                  std::is_convertible<InnerExecutor1, InnerExecutor>,
                          std::false_type
          >::type::value>::type = 0) noexcept
      : InnerExecutor(ex)
    {
    }
  };

  /// Type alias to adapt an I/O object to use @c deferred_t as its
  /// default completion token type.
  template <typename T>
  using as_default_on_t = typename T::template rebind_executor<
        executor_with_default<typename T::executor_type> >::other;

  /// Function helper to adapt an I/O object to use @c deferred_t as its
  /// default completion token type.
  template <typename T>
  static typename std::decay_t<T>::template rebind_executor<
      executor_with_default<typename std::decay_t<T>::executor_type>
    >::other
  as_default_on(T && object)
  {
    return typename std::decay_t<T>::template rebind_executor<
        executor_with_default<typename std::decay_t<T>::executor_type>
      >::other(std::forward<T>(object));
  }

};

constexpr use_eager_op_t use_eager_op{};

}

namespace boost::asio
{

template<typename ... Args>
struct async_result<boost::async::use_eager_op_t, void(Args...)>
{
  template <typename Initiation, typename... InitArgs>
  struct eager_op_impl : boost::async::eager_op<Args...>
  {
    Initiation initiation;
    std::tuple<InitArgs...> args;
    template<typename Initiation_, typename ...InitArgs_>
    eager_op_impl(Initiation_ && initiation,
            InitArgs_   && ... args)
            : initiation(std::forward<Initiation_>(initiation))
            , args(std::forward<InitArgs_>(args)...) {}

    void initiate(async::completion_handler<Args...> complete) override
    {
      std::apply(
          [&](InitArgs && ... args)
          {
            std::move(initiation)(std::move(complete),
                                  std::forward<InitArgs>(args)...);
          }, std::move(args));
    }
  };

  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation && initiation,
                       boost::async::use_eager_op_t,
                       InitArgs &&... args)
      -> eager_op_impl<Initiation, InitArgs...>
  {
    return eager_op_impl<Initiation, InitArgs...>(
        std::forward<Initiation>(initiation),
        std::forward<InitArgs>(args)...);
  }
};
}
#endif //BOOST_ASYNC_EAGER_OP_HPP
