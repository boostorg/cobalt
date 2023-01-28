//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_IMPL_EXCEPTION_IPP
#define BOOST_ASYNC_DETAIL_IMPL_EXCEPTION_IPP

#include <boost/async/detail/exception.hpp>
#include <stdexcept>

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

}

#endif //BOOST_ASYNC_DETAIL_IMPL_EXCEPTION_IPP
