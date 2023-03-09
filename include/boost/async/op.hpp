//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_OP_HPP
#define BOOST_ASYNC_OP_HPP

#include <boost/async/detail/op.hpp>

namespace boost::async
{

template<typename T, auto = &T::initiate>
auto operator co_await(T && t) -> detail::deferred_op<std::decay_t<T>>
{
  return detail::deferred_op<T>{std::forward<T>(t)};
}

template<typename T>
struct enable_op
{
  auto operator co_await() && -> detail::deferred_op<std::decay_t<T>>
  {
    return detail::deferred_op<T>{std::move(*static_cast<T*>(this))};
  }

  auto operator co_await() & -> detail::deferred_op<std::decay_t<T>>
  {
    return detail::deferred_op<T>{*static_cast<T*>(this)};
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

constexpr use_op_t use_op{};

}

namespace boost::asio
{

template<typename ... Args>
struct async_result<boost::async::use_op_t, void(Args...)>
{
  template <typename Initiation, typename... InitArgs>
  struct op_impl
  {
    Initiation initiation;
    std::tuple<InitArgs...> args;

    void initiate(async::completion_handler<Args...> complete)
    {
      std::apply(
          [&](InitArgs && ... args)
          {
            std::move(initiation)(std::move(complete), std::forward<InitArgs>(args)...);
          }, std::move(args));
    }
  };

  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation && initiation,
                       boost::async::use_op_t,
                       InitArgs &&... args)
      -> boost::async::detail::deferred_op<op_impl<Initiation, std::decay_t<InitArgs>...>>
  {
    return
      boost::async::detail::deferred_op<op_impl<Initiation, std::decay_t<InitArgs>...>>
        ({
          std::forward<Initiation>(initiation),
          {std::forward<InitArgs>(args)...}
        });
  }
};

}

#endif //BOOST_ASYNC_OP_HPP
