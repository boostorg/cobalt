//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/signal_set.hpp>

namespace boost::async::io
{

signal_set::signal_set() : signal_set_(this_thread::get_executor()) {}
signal_set::signal_set(std::initializer_list<int> sigs)
  : signal_set_(this_thread::get_executor())
{
    for (auto i : sigs)
      add(i).value();
}

system::result<void> signal_set::cancel()
{
  system::error_code ec;
  signal_set_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> signal_set::clear()
{
  system::error_code ec;
  signal_set_.clear(ec);
  return ec ? ec : system::result<void>{};
}
system
::result<void> signal_set::add(int signal_number)
{
  system::error_code ec;
  signal_set_.add(signal_number, ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> signal_set::remove(int signal_number)
{
  system::error_code ec;
  signal_set_.remove(signal_number, ec);
  return ec ? ec : system::result<void>{};
}

void signal_set::wait_op_::init_op(completion_handler<system::error_code, int> handler)
{
  signal_set_.async_wait(std::move(handler));
}

}