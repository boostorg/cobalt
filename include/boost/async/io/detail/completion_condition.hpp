//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_COMPLETION_CONDITION_HPP
#define BOOST_ASYNC_IO_COMPLETION_CONDITION_HPP

#include <boost/system/error_code.hpp>

namespace boost::async::io::detail
{

struct completion_condition_base
{
  virtual std::size_t operator()(const system::error_code & ec,std::size_t bytes_transferred) = 0;
};

struct completion_condition
{
  std::size_t operator()(const system::error_code & ec,std::size_t bytes_transferred)
  {
    return (*cc)(ec, bytes_transferred);
  }
  completion_condition_base * cc;
};


}

#endif //BOOST_ASYNC_IO_COMPLETION_CONDITION_HPP
