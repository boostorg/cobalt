//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/io/signal_set.hpp>

namespace boost::cobalt::io
{

signal_set::signal_set(const cobalt::executor & executor) : signal_set_(executor) {}
signal_set::signal_set(std::initializer_list<int> sigs, const cobalt::executor & executor)
    : signal_set_(executor)
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

system::result<void> signal_set::add(int signal_number)
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

void signal_set::wait_op_::initiate(completion_handler<system::error_code, int> h)
{
  signal_set_.async_wait(std::move(h));
}

}