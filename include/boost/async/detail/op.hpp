//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_OP_HPP
#define BOOST_ASYNC_DETAIL_OP_HPP

#include <boost/async/detail/handler.hpp>

#include <boost/asio/io_context.hpp>

namespace boost::async::detail
{

template<typename Class, typename Handler>
constexpr auto deduce_initiate_handler(void (Class::* const)(Handler)) -> typename std::decay_t<Handler>::result_type;

template<typename Class, typename Handler>
constexpr auto deduce_initiate_handler(void (Class::* const)(Handler) const) -> typename std::decay_t<Handler>::result_type;


template<typename Op>
struct [[nodiscard]] deferred_op
{
  Op op;
  std::exception_ptr error;
  using result_type = decltype(deduce_initiate_handler(&Op::initiate));
  result_type result;

  constexpr static bool await_ready()
    requires (not requires {&Op::ready;})
  {return false;}

  bool await_ready() requires requires {{op.ready(result)};}
  {
    op.ready(result);
    return result.has_value();
  }

  bool await_ready()
    requires std::is_same_v<typename result_type::value_type, std::tuple<system::error_code>>
      && requires (system::error_code & ec) {{op.ready(ec)} -> std::convertible_to<bool>;}
  {
    system::error_code ec;
    if (op.ready(ec))
    {
      result.emplace(ec);
      return true;
    }
    else
      return false;
  }


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      op.initiate({h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  auto await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    return interpret_result(*std::move(result));
  }
};

}

#endif //BOOST_ASYNC_DETAIL_OP_HPP
