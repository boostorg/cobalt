//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/config.hpp>

#ifndef BOOST_ASYNC_CONFIG_HPP
#define BOOST_ASYNC_CONFIG_HPP

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_ASYNC_DYN_LINK)
#if defined(BOOST_ASYNC_SOURCE)
#define BOOST_ASYNC_DECL BOOST_SYMBOL_EXPORT
#else
#define BOOST_ASYNC_DECL BOOST_SYMBOL_IMPORT
#endif
#else
#define BOOST_ASYNC_DECL
#endif

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

using executor = BOOST_ASYNC_EXECUTOR;

}

# define BOOST_ASYNC_ERR(ev) ( \
    ::boost::system::error_code( (ev), [] { \
    static constexpr auto loc((BOOST_CURRENT_LOCATION)); \
    return &loc; }()))
# define BOOST_ASYNC_RETURN_EC(ev)                                     \
    do {                                                               \
      static constexpr auto loc ## __LINE__((BOOST_CURRENT_LOCATION)); \
      return ::boost::system::error_code((ev), &loc ## __LINE__);      \
    } while(false)


#endif //BOOST_ASYNC_CONFIG_HPP
