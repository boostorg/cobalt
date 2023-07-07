//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_SIGNAL_SET_HPP
#define BOOST_ASYNC_IO_SIGNAL_SET_HPP

#include <boost/async/io/result.hpp>

#include <boost/asio/basic_signal_set.hpp>
#include <boost/system/result.hpp>

#include <initializer_list>

namespace boost::async::io
{

struct signal_set
{
  using wait_result = system::result<int>;

  BOOST_ASYNC_DECL signal_set();
  BOOST_ASYNC_DECL signal_set(std::initializer_list<int> sigs);

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> cancel();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> clear();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> add(int signal_number);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> remove(int signal_number);


 private:
  struct wait_op_ final : result_op<int>
  {
    BOOST_ASYNC_DECL void initiate(completion_handler<system::error_code, int> handler) override;
    wait_op_(boost::asio::basic_signal_set<executor> & signal_set) : signal_set_(signal_set) {}
   private:
    boost::asio::basic_signal_set<executor> & signal_set_;
  };
 public:
  [[nodiscard]] wait_op_ wait() { return wait_op_{signal_set_}; }
  wait_op_ operator co_await () { return wait(); }
 private:
  boost::asio::basic_signal_set<executor> signal_set_;
};


}

#endif //BOOST_ASYNC_IO_SIGNAL_SET_HPP
