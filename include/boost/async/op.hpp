//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_OP_HPP
#define BOOST_ASYNC_OP_HPP

#include <asio/io_context.hpp>
#include <handler.hpp>

namespace boost::async
{

struct timer_wait_op
{
  struct base
  {
    virtual bool await_ready(void * p) const = 0;
    virtual void await_resume(void * p,
                              completion_handler_type<
                                  asio::cancellation_slot,
                                  typename asio::io_context::executor_type,
                                  std::pmr::polymorphic_allocator<void>> h) const = 0;
  };

  void * impl;
  const base & methods;
  std::optional<std::tuple<asio::error_code>> result;
  std::exception_ptr error;

  bool await_ready() const {return methods.await_ready(impl);}


  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      methods.await_resume(impl, {h, result});
      return true;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  void await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    if (std::get<0>(*result))
      throw asio::system_error(std::get<0>(*result));
  }
};

}

#endif //BOOST_ASYNC_OP_HPP
