//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/detail/exception.hpp>

namespace boost::async::detail
{

std::exception_ptr moved_from_exception()
{
  static auto ep = std::make_exception_ptr(std::logic_error("async::promise was moved from"));
  return ep;
}

std::exception_ptr detached_exception()
{
  static auto ep = std::make_exception_ptr(std::runtime_error("detached"));
  return ep;
}

std::exception_ptr completed_unexpected()
{
  static auto ep = std::make_exception_ptr(std::runtime_error("unexpected exit"));
  return ep;
}

std::exception_ptr wait_not_ready()
{
  static auto ep = std::make_exception_ptr(std::runtime_error("wait_not_ready"));
  return ep;
}

std::exception_ptr already_awaited()
{
    static auto ep = std::make_exception_ptr(std::runtime_error("already-awaited"));
    return ep;
}

void throw_already_awaited(const boost::source_location & loc)
{
  throw_exception(std::runtime_error("already-awaited"), loc);
}


}
