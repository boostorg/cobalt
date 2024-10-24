//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP

#include <boost/cobalt/concepts.hpp>
#include <boost/cobalt/detail/await_result_helper.hpp>
#include <boost/cobalt/experimental/io/buffer.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/result.hpp>

namespace boost::cobalt::experimental::io
{

template<typename Awaitable, typename ... Ts>
concept io_op =
  awaitable<Awaitable> &&
  std::same_as<
      detail::co_await_result_t<as_tuple_t<decltype(detail::get_awaitable_type(std::declval<Awaitable>()))>>,
      std::tuple<Ts...>>;



template<typename Awaitable>
concept transfer_op = io_op<Awaitable, system::error_code, std::size_t>;



template<typename Op, typename Args, typename ... Ts>
struct op_awaitable_base
{
  Op & op_;
  Args args;
  std::optional<std::tuple<Ts...>> result;

#if !defined(BOOST_COBALT_NO_PMR)
  using resource_type = pmr::memory_resource;
#else
  using resource_type = detail::sbo_resource;
#endif

  template<typename ... Args_>
  op_awaitable_base(resource_type *resource, Op * op_, Args_ && ... args) : op_(*op_), args(std::forward<Args_>(args)...), resource(resource) {}
  op_awaitable_base(op_awaitable_base && lhs) : op_(lhs.op_), args(std::move(lhs.args)), result(std::move(lhs.result))
  {
  }

  bool await_ready() { return false; }

  detail::completed_immediately_t completed_immediately = detail::completed_immediately_t::no;
  std::exception_ptr init_ep;

  resource_type *resource;

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h
#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
      , const boost::source_location & loc = BOOST_CURRENT_LOCATION
#endif
  ) noexcept
  {
    BOOST_TRY
      {
        completed_immediately = detail::completed_immediately_t::initiating;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
        completion_handler<Ts...> ch{h, result, resource, &completed_immediately, loc};
#else
        completion_handler<Ts...> ch{h, result, resource, &completed_immediately};
#endif
        std::apply([&]<typename ... Args_>(Args_ && ... args_)
                   {
                      (*op_.implementation)(op_.this_, std::forward<Args_>(args_)..., std::move(ch));
                   }, std::move(args));

        if (completed_immediately == detail::completed_immediately_t::initiating)
          completed_immediately = detail::completed_immediately_t::no;
        return completed_immediately != detail::completed_immediately_t::yes;
      }
      BOOST_CATCH(...)
      {
        init_ep = std::current_exception();
        return false;
      }
    BOOST_CATCH_END
  }

  auto await_resume(const struct as_tuple_tag &)
  {
    if (init_ep)
      std::rethrow_exception(init_ep);
    return *std::move(result);
  }

  auto await_resume(const struct as_result_tag &)
  {
    if (init_ep)
      std::rethrow_exception(init_ep);
    return interpret_as_result(*std::move(result));
  }

  auto await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    if (init_ep)
      std::rethrow_exception(init_ep);
    return await_resume(as_result_tag{}).value(loc);
  }

};

template<typename Op, typename Args, typename ... Ts>
struct op_awaitable : op_awaitable_base<Op, Args, Ts...>
{
  char buffer[BOOST_COBALT_SBO_BUFFER_SIZE];
  detail::sbo_resource resource{buffer, sizeof(buffer)};

  template<typename ... Args_>
  op_awaitable(Op * op_, Args_ && ... args) : op_awaitable_base<Op, Args, Ts...>(&resource, op_, std::forward<Args_>(args)...) {}
  op_awaitable(op_awaitable && lhs) : op_awaitable_base<Op, Args, Ts...>(std::move(lhs))
  {
    this->op_awaitable_base<Op, Args, Ts...>::resource = &resource;
  }

  op_awaitable_base<Op, Args, Ts...> replace_resource(typename op_awaitable_base<Op, Args, Ts...>::resource_type * resource) &&
  {
    op_awaitable_base<Op, Args, Ts...> nw = std::move(*this);
    nw.resource = resource;
    return nw;
  }
};

struct write_op
{
  const_buffer_sequence buffer;

  void *this_;
  void (*implementation)(void * this_, const_buffer_sequence,
                         boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  op_awaitable<write_op, std::tuple<const_buffer_sequence>, boost::system::error_code, std::size_t>
  operator co_await()
  {
    return {this, buffer};
  }
};

struct read_op
{
  mutable_buffer_sequence buffer;

  void *this_;
  void (*implementation)(void * this_, mutable_buffer_sequence,
                         boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  op_awaitable<read_op, std::tuple<mutable_buffer_sequence>, boost::system::error_code, std::size_t>
      operator co_await()
  {
    return {this, buffer};
  }
};

struct write_at_op
{
  std::uint64_t offset;
  const_buffer_sequence buffer;

  void *this_;
  void (*implementation)(void * this_, std::uint64_t, const_buffer_sequence,
                         boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  op_awaitable<write_at_op, std::tuple<std::uint64_t , const_buffer_sequence>, boost::system::error_code, std::size_t>
      operator co_await()
  {
    return {this, offset, buffer};
  }
};

struct read_at_op
{
  std::uint64_t offset;
  mutable_buffer_sequence buffer;

  void *this_;
  void (*implementation)(void * this_, std::uint64_t, mutable_buffer_sequence,
                         boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  op_awaitable<read_at_op, std::tuple<std::uint64_t , mutable_buffer_sequence>, boost::system::error_code, std::size_t>
      operator co_await()
  {
    return {this, offset, buffer};
  }
};

struct wait_op
{
  void *this_;
  void (*implementation)(void * this_,
                         boost::cobalt::completion_handler<boost::system::error_code>);

  op_awaitable<wait_op, std::tuple<>, boost::system::error_code>
      operator co_await()
  {
    return {this};
  }
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_OPS_HPP
