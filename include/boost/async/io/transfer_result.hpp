//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_TRANSFER_RESULT_HPP
#define BOOST_ASYNC_TRANSFER_RESULT_HPP

#include <boost/async/config.hpp>
#include <boost/async/detail/handler.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/result.hpp>


namespace boost::async::io
{

struct [[nodiscard]] transfer_result
{
  system::error_code error;
  std::size_t transferred{0u};

  using value_type = std::size_t;
  using error_type = system::error_code;

  // queries
  constexpr bool has_value() const noexcept { return transferred != 0; }
  constexpr bool has_error() const noexcept { return error.failed(); }
  constexpr explicit operator bool() const noexcept { return has_value() || !has_error(); }
  constexpr std::size_t value( boost::source_location const& loc = BOOST_CURRENT_LOCATION ) const noexcept
  {
    if (!has_value() || has_error())
      throw_exception_from_error(error, loc);
    return transferred;
  }
  constexpr std::size_t operator*() const noexcept { BOOST_ASSERT(has_value()); return transferred; }

  bool operator==(const system::error_code &ec) const { return error == ec;}
  bool operator!=(const system::error_code &ec) const { return error != ec;}

  auto operator<=>(std::size_t n) const { return value() <=> n;}
  bool operator==(std::size_t n) const { return value() == n;}

  template<typename E>
      requires system::is_error_code_enum<E>::value
  bool operator==(E e) const { return error == e;}

  template<typename E>
      requires system::is_error_code_enum<E>::value
  bool operator!=(E e) const { return error != e;}
};

inline transfer_result & operator+=(transfer_result & lhs, const transfer_result & rhs)
{
  lhs.transferred += rhs.transferred;
  if (!lhs.error)
    lhs.error = rhs.error;
  return lhs;
}

struct transfer_op
{
  virtual void ready(async::handler<system::error_code, std::size_t> h) {};
  virtual void initiate(async::completion_handler<system::error_code, std::size_t> complete) = 0 ;
  virtual ~transfer_op() = default;

  transfer_op() = default;
  transfer_op(transfer_op && lhs)
      : error(std::move(lhs.error))
      , result(std::move(lhs.result))
  {
    BOOST_ASSERT(!lhs.resource);
  }

  constexpr static bool await_ready()
  {
    return false;
  }

  template<typename Promise>
  bool await_suspend(std::coroutine_handle<Promise> h)
  {
    try
    {
      auto & res = resource.emplace(buffer, sizeof(buffer),
                                    asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
      completed_immediately = detail::completed_immediately_t::initiating;
      initiate(completion_handler<system::error_code, std::size_t>{h, result, &res, &completed_immediately});
      if (completed_immediately == detail::completed_immediately_t::initiating)
        completed_immediately = detail::completed_immediately_t::no;
      return completed_immediately != detail::completed_immediately_t::yes;
    }
    catch(...)
    {
      error = std::current_exception();
      return false;
    }
  }

  [[nodiscard]] transfer_result await_resume()
  {
    if (error)
      std::rethrow_exception(std::exchange(error, nullptr));

    return transfer_result{std::get<0>(*result), std::get<1>(*result)};
  }

  struct vawaitable
  {
    vawaitable(transfer_op * op_) : op_(op_) {}

    bool await_ready() { return op_->await_ready();}

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      return op_->await_suspend(h);
    }

    std::size_t await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      return op_->await_resume().value(loc);
    }
   private:
    transfer_op * op_;
  };

  vawaitable value() { return vawaitable{this}; }
 private:
  std::exception_ptr error;
  std::optional<std::tuple<system::error_code, std::size_t>> result;
  char buffer[2048];
  std::optional<pmr::monotonic_buffer_resource> resource;
  detail::completed_immediately_t completed_immediately = detail::completed_immediately_t::no;
};



}

#endif //BOOST_ASYNC_TRANSFER_RESULT_HPP
