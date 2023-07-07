//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_RESULT_HPP
#define BOOST_ASYNC_IO_RESULT_HPP

#include <boost/async/config.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <boost/system/result.hpp>

namespace boost::async::io
{

template<typename T = void, typename Error = system::error_code>
struct result_op
{
  using value_type = T;
  using error_type = Error;

  virtual void ready(async::handler<Error, T> h) {};
  virtual void initiate(async::completion_handler<Error, T> complete) = 0 ;
  virtual ~result_op() = default;

  result_op() = default;
  result_op(result_op && lhs)
      : error(std::move(lhs.error))
      , result(std::move(lhs.result))
  {
    BOOST_ASSERT(!lhs.resource);
  }

  bool await_ready()
  {
    ready(handler<Error, T>(result));
    return result.has_value();
  }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(buffer, sizeof(buffer),
                                    asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      initiate(completion_handler<Error, T>{h, result, &res, &completed_immediately});
      return !completed_immediately;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] system::result<T, Error> await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    if (std::get<0>(*this->result))
      return system::result<T, Error>(system::in_place_error, std::get<0>(std::move(*this->result)));
    else
      return system::result<T, Error>(system::in_place_value, std::get<1>(std::move(*this->result)));
  }

  struct vawaitable
  {
    vawaitable(result_op<T, Error> * op_) : op_(op_) {}

    bool await_ready() { return op_->await_ready();}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      return op_->await_suspend(h);
    }

    T await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      return op_->await_resume().value(loc);
    }
   private:
    result_op<T, Error> * op_;
  };

  vawaitable value() && { return vawaitable{this}; }
 private:
  std::exception_ptr error;
  std::optional<std::tuple<Error, T>> result;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
  bool completed_immediately = false;
};



template<typename Error>
struct result_op<void, Error>
{
  using value_type = void;
  using error_type = Error;

  virtual void ready(async::handler<Error> h) {};
  virtual void initiate(async::completion_handler<Error> complete) = 0 ;
  virtual ~result_op() = default;

  result_op() noexcept = default;
  result_op(result_op && lhs)
    : error(std::move(lhs.error))
    , result(std::move(lhs.result))
  {
    BOOST_ASSERT(!lhs.resource);
  }

  bool await_ready()
  {
    ready(handler<Error>(result));
    return result.has_value();
  }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(buffer, sizeof(buffer),
                                    asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      initiate(completion_handler<Error>{h, result, &res, &completed_immediately});
      return !completed_immediately;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] system::result<void, Error> await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));
    if (std::get<0>(*this->result))
      return system::result<void, Error>(system::in_place_error, std::get<0>(std::move(*this->result)));
    else
      return system::in_place_value;

  }

  struct vawaitable
  {
    vawaitable(result_op<void, Error> * op_) : op_(op_) {}

    bool await_ready() { return op_->await_ready();}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      return op_->await_suspend(h);
    }

    void await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      op_->await_resume().value(loc);
    }
   private:
    result_op<void, Error> * op_;
  };

  vawaitable value() && { return vawaitable{this}; }

 private:
  std::exception_ptr error;
  std::optional<std::tuple<Error>> result;
  char buffer[2048];
  std::optional<container::pmr::monotonic_buffer_resource> resource;
  bool completed_immediately = false;
};


}

#endif //BOOST_ASYNC_IO_RESULT_HPP
