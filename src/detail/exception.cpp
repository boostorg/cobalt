//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/detail/exception.hpp>
#include <boost/cobalt/error.hpp>

#include <boost/asio/executor.hpp>

namespace boost::cobalt::detail
{

std::exception_ptr moved_from_exception()
{
   static auto ep = std::make_exception_ptr(system::system_error(
      error::moved_from
      ));
  return ep;
}

std::exception_ptr detached_exception()
{

  static auto ep = std::make_exception_ptr(system::system_error(
        error::detached
      ));
  return ep;
}

std::exception_ptr completed_unexpected()
{

  static auto ep = std::make_exception_ptr(system::system_error(
          error::completed_unexpected
      ));
  return ep;
}

std::exception_ptr wait_not_ready()
{
  static auto ep = std::make_exception_ptr(system::system_error(
          error::wait_not_ready
      ));
  return ep;
}

std::exception_ptr already_awaited()
{
  static auto ep = std::make_exception_ptr(system::system_error(
        error::already_awaited
    ));
  return ep;
}


std::exception_ptr allocation_failed()
{
  static auto ep = std::make_exception_ptr(system::system_error(
      error::already_awaited
  ));
  return ep;
}

void throw_bad_executor(const boost::source_location & loc)
{
#if defined(BOOST_ASIO_NO_TS_EXECUTORS)
  boost::throw_exception(boost::asio::execution::bad_executor(), loc);
#else
  boost::throw_exception(boost::asio::bad_executor(), loc);
#endif
}


}
