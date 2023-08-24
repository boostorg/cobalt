// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_ERROR_HPP
#define BOOST_ASYNC_ERROR_HPP

#include <boost/async/config.hpp>
#include <boost/system/error_code.hpp>

namespace boost::async
{

enum class error
{
  moved_from,
  detached,
  completed_unexpected,
  wait_not_ready,
  already_awaited,
  allocation_failed
};


struct async_category_t final : system::error_category
{
  async_category_t() : system::error_category(0x7d4c7b49d8a4fdull) {}


  std::string message( int ev ) const override
  {

    return message(ev, nullptr, 0u);
  }
  char const * message( int ev, char * , std::size_t ) const noexcept override
  {
    switch (static_cast<error>(ev))
    {
      case error::moved_from:
        return "moved from";
      case error::detached:
        return "detached";
      case error::completed_unexpected:
        return "completed unexpected";
      case error::wait_not_ready:
        return "wait not ready";
      case error::already_awaited:
        return "already awaited";
      case error::allocation_failed:
        return "allocation failed";
      default:
        return "unknown async error";
    }
  }

  const char * name() const BOOST_NOEXCEPT override
  {
    return "boost.async";
  }
};

BOOST_ASYNC_DECL system::error_category & async_category();
BOOST_ASYNC_DECL system::error_code make_error_code(error e);

}

template<> struct boost::system::is_error_code_enum<boost::async::error>
{
  static const bool value = true;
};

#endif //BOOST_ASYNC_ERROR_HPP
