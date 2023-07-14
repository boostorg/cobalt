// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/async/error.hpp>

namespace boost::async
{
system::error_category & async_category()
{
  static async_category_t cat;
  return cat;
}

system::error_code make_error_code(error e)
{
  return system::error_code(static_cast<int>(e), async_category());
}

}

