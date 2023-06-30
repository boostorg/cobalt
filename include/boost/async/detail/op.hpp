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

struct deferred_op_resource_base
{
  deferred_op_resource_base() noexcept = default;
  deferred_op_resource_base(deferred_op_resource_base && lhs) noexcept
  {
    BOOST_ASSERT(!lhs.resource);
  }

  template<typename Promise>
  container::pmr::monotonic_buffer_resource * get_resource(std::coroutine_handle<Promise> h)
  {
    return &resource.emplace(
        buffer, sizeof(buffer),
        asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource()
        );
  }

 private:
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
};

template<typename Op>
struct [[nodiscard]] deferred_op : deferred_op_resource_base
{
  Op op;
  std::exception_ptr error;
  using result_type = decltype(deduce_initiate_handler(&Op::initiate));
  result_type result;

  template<typename Op_>
  deferred_op(Op_ && op_) : op(std::forward<Op_>(op_)) {}

  deferred_op(      Op && op) : op(std::move(op)) {}
  deferred_op(const Op  & op) : op(op) {}


  inline deferred_op(deferred_op && lhs)
    : op(std::move(lhs.op))
    , error(std::move(lhs.error))
    , result(std::move(lhs.result))
  {
    BOOST_ASSERT(!lhs.resource);
  }

  bool await_ready()
  {
    if constexpr (! requires {&Op::ready;})
      return false;
    else if constexpr (requires {{op.ready(result)};})
    {
      op.ready(result);
      return result.has_value();
    }
    else if constexpr (
        std::is_same_v<typename result_type::value_type, std::tuple<system::error_code>>
                   && requires (system::error_code & ec) {{op.ready(ec)} -> std::convertible_to<bool>;})
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
    else
      static_assert(std::is_same_v<Op, void>, "Invalid ready-call");

  }

  bool completed_immediately = false;
  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      op.initiate({h, result, this->get_resource(h), &completed_immediately});
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
