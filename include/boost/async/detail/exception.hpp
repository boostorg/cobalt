//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_EXCEPTION_HPP
#define BOOST_ASYNC_DETAIL_EXCEPTION_HPP

#include <exception>

namespace boost::async::detail
{

std::exception_ptr moved_from_exception();
std::exception_ptr detached_exception();
std::exception_ptr completed_unexpected();

}

#endif //BOOST_ASYNC_DETAIL_EXCEPTION_HPP
