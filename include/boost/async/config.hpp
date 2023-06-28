//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_CONFIG_HPP
#define BOOST_ASYNC_CONFIG_HPP

#if !defined(BOOST_ASYNC_EXECUTOR)
# if defined(BOOST_ASYNC_USE_ANY_IO_EXECUTOR)
#  include <boost/asio/any_io_executor.hpp>
#  define BOOST_ASYNC_EXECUTOR boost::asio::any_io_executor
# else
#  include <boost/asio/io_context.hpp>
#  define BOOST_ASYNC_EXECUTOR boost::asio::io_context::executor_type
# endif
#endif


namespace boost::async
{

using executor_type = BOOST_ASYNC_EXECUTOR;


}

#endif //BOOST_ASYNC_CONFIG_HPP
