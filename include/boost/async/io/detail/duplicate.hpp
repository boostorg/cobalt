//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_DUPLICATE_HPP
#define BOOST_ASYNC_IO_DUPLICATE_HPP

#include <boost/asio/detail/config.hpp>
#include <boost/async/config.hpp>
#include <boost/system/result.hpp>

namespace boost::async::detail::io
{

#if defined(BOOST_ASIO_WINDOWS)
BOOST_ASYNC_DECL system::result<HANDLE> duplicate_handle(HANDLE h);
BOOST_ASYNC_DECL system::result<SOCKET  > duplicate_socket(SOCKET fd);
#else
BOOST_ASYNC_DECL system::result<int> duplicate_handle(int fd);
BOOST_ASYNC_DECL system::result<int> duplicate_socket(int fd);
#endif

}

#endif //BOOST_ASYNC_IO_DUPLICATE_HPP
