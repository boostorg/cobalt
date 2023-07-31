//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_OP_HPP
#define BOOST_ASYNC_OP_HPP

#include <boost/async/detail/handler.hpp>



namespace boost::async
{


template<typename ... Args>
struct op
{
  virtual void ready(async::handler<Args...> h) {};
  virtual void initiate(async::completion_handler<Args...> complete) = 0 ;
  virtual ~op() = default;

  struct awaitable
  {
    op<Args...> &op_;
    std::optional<std::tuple<Args...>> result;

    awaitable(op<Args...> * op_) : op_(*op_) {}
    awaitable(awaitable && lhs)
        : op_(lhs.op_)
        , result(std::move(lhs.result))
    {
      BOOST_ASSERT(!lhs.resource);
    }

    bool await_ready()
    {
      op_.ready(handler<Args...>(result));
      return result.has_value();
    }

    char buffer[2048];
    std::optional<pmr::monotonic_buffer_resource> resource;
    detail::completed_immediately_t completed_immediately = detail::completed_immediately_t::no;
    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h,
                       const boost::source_location & loc = BOOST_CURRENT_LOCATION) noexcept
    {
      std::optional<executor> exec;
      if (!this_thread::has_executor())
        exec = get_executor(h);
      try
      {
        auto & res = resource.emplace(buffer, sizeof(buffer),
                                      asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
        op_.initiate(completion_handler<Args...>{h, result, &res, &completed_immediately});
        return completed_immediately != detail::completed_immediately_t::yes;
      }
      catch(...)
      {
        asio::post(exec.value_or(async::this_thread::get_executor()),
                   [e = std::current_exception()]{std::rethrow_exception(e);});
        return true;
      }
    }

    auto await_resume()
    {
      return interpret_result(*std::move(result));
    }


  };

  awaitable operator co_await() &&
  {
    return awaitable{this};
  }
};

struct use_op_t
{
  /// Default constructor.
  constexpr use_op_t()
  {
  }

  /// Adapts an executor to add the @c deferred_t completion token as the
  /// default.
  template <typename InnerExecutor>
  struct executor_with_default : InnerExecutor
  {
    /// Specify @c deferred_t as the default completion token type.
    typedef use_op_t default_completion_token_type;

    executor_with_default(const InnerExecutor& ex) noexcept
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

constexpr use_op_t use_op{};

}

namespace boost::asio
{

template<typename ... Args>
struct async_result<boost::async::use_op_t, void(Args...)>
{
  template <typename Initiation, typename... InitArgs>
  struct op_impl final : boost::async::op<Args...>
  {
    Initiation initiation;
    std::tuple<InitArgs...> args;
    template<typename Initiation_, typename ...InitArgs_>
    op_impl(Initiation_ initiation,
            InitArgs_   && ... args)
            : initiation(std::move(initiation))
            , args(std::move(args)...) {}

    void initiate(async::completion_handler<Args...> complete) final override
    {
      std::apply(
          [&](InitArgs && ... args)
          {
            std::move(initiation)(std::move(complete),
                                  std::move(args)...);
          }, std::move(args));
    }
  };

  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation && initiation,
                       boost::async::use_op_t,
                       InitArgs &&... args)
      -> op_impl<std::decay_t<Initiation>, std::decay_t<InitArgs>...>
  {
    return op_impl<std::decay_t<Initiation>, std::decay_t<InitArgs>...>(
        std::forward<Initiation>(initiation),
        std::forward<InitArgs>(args)...);
  }
};
}
#endif //BOOST_ASYNC_OP_HPP
