//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/detail/sbo_resource.hpp>

#include "doctest.h"

TEST_SUITE_BEGIN("sbo_resource");

TEST_CASE("basic")
{
  char buf[1024];
  boost::cobalt::detail::sbo_resource res{buf, sizeof(buf)};

  {
    std::vector<int, boost::cobalt::detail::sbo_allocator<int>> vec{&res};

    for (int i = 0u; i < 4000; i++)
      vec.push_back(i);
  }
}

TEST_CASE("too-small")
{
  char buf[1];
  boost::cobalt::detail::sbo_resource res{buf, sizeof(buf)};

  {
    std::vector<int, boost::cobalt::detail::sbo_allocator<int>> vec{&res};

    for (int i = 0u; i < 4000; i++)
      vec.push_back(i);
  }
}

TEST_CASE("no-buf")
{
  boost::cobalt::detail::sbo_resource res;
  {
    std::vector<int, boost::cobalt::detail::sbo_allocator<int>> vec{&res};

    for (int i = 0u; i < 4000; i++)
      vec.push_back(i);
  }
}

TEST_SUITE_END();