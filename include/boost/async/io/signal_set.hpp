//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SIGNAL_SET_HPP
#define BOOST_ASYNC_IO_SIGNAL_SET_HPP

#include <boost/async/op.hpp>

#include <boost/asio/basic_signal_set.hpp>
#include <boost/system/result.hpp>

#include <initializer_list>

namespace boost::async::io
{

struct signal_set
{
  using wait_result = system::result<int>;

  signal_set();
  signal_set(std::initializer_list<int> sigs);

  [[nodiscard]] system::result<void> cancel();
  [[nodiscard]] system::result<void> clear();
  [[nodiscard]] system::result<void> add(int signal_number);
  [[nodiscard]] system::result<void> remove(int signal_number);


 private:
  struct wait_op_
  {
    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h)
    {
      try
      {
        auto & res = resource.emplace(
            buffer, sizeof(buffer),
            asio::get_associated_allocator(h.promise(), this_thread::get_allocator()).resource());
        signal_set_.async_wait(completion_handler<system::error_code, int>{h, result_, &res});
        return true;
      }
      catch(...)
      {
        error = std::current_exception();
        return false;
      }
    }

    [[nodiscard]] wait_result await_resume()
    {
      if (error)
        std::rethrow_exception(std::exchange(error, nullptr));
      auto [ec, sig] = result_.value_or(std::make_tuple(system::error_code{}, 0));
      if (ec)
        return ec;
      else
        return sig;
    }
    wait_op_(boost::asio::basic_signal_set<asio::io_context::executor_type> & signal_set) : signal_set_(signal_set) {}
   private:
    boost::asio::basic_signal_set<asio::io_context::executor_type> & signal_set_;
    std::exception_ptr error;
    std::optional<std::tuple<system::error_code, int>> result_;
    char buffer[256];
    std::optional<container::pmr::monotonic_buffer_resource> resource;
  };
 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{signal_set_}; }
  wait_op_ operator co_await () { return wait(); }
 private:
  boost::asio::basic_signal_set<asio::io_context::executor_type> signal_set_;
};


}

#endif //BOOST_ASYNC_IO_SIGNAL_SET_HPP
