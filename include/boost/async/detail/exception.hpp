//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_EXCEPTION_HPP
#define BOOST_ASYNC_DETAIL_EXCEPTION_HPP

#include <exception>
#include <stdexcept>

namespace boost::async::detail
{

inline std::exception_ptr moved_from_exception()
{
  static auto ep = std::make_exception_ptr(std::logic_error("async::promise was moved from"));
  return ep;
}


}

#endif //BOOST_ASYNC_DETAIL_EXCEPTION_HPP
