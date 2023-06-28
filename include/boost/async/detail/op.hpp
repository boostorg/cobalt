//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_OP_HPP
#define BOOST_ASYNC_DETAIL_OP_HPP

#include <boost/async/detail/handler.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

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

  template<typename Op_>
  deferred_op(Op_ && op_) : op(std::forward<Op_>(op_)) {}

  deferred_op(      Op && op) : op(std::move(op)) {}
  deferred_op(const Op  & op) : op(op) {}


  deferred_op(deferred_op && lhs)
    : op(std::move(lhs.op))
    , error(std::move(lhs.error))
    , result(std::move(lhs.result))
  {
    assert(!lhs.resource);
  }

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

  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
  bool completed_immediately = false;
  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(
          buffer, sizeof(buffer),
          asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      op.initiate({h, result, &res, &completed_immediately});
      return !completed_immediately;
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
