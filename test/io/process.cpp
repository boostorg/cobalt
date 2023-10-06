//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/process.hpp>

#include <random>

using namespace boost::async;
namespace bpv = boost::process::v2;

#if defined(BOOST_PROCESS_V2_WINDOWS)
bpv::filesystem::path shell()
{
  return bpv::environment::find_executable("cmd");
}
#else
bpv::filesystem::path shell()
{
  return bpv::environment::find_executable("sh");
}
#endif

CO_TEST_CASE("process")
{

#if defined(BOOST_PROCESS_V2_WINDOWS)
  CHECK(42 == co_await io::process(bpv::environment::find_executable("cmd"), {"/c", "exit 42"}));

#else
  CHECK(42 == co_await io::process(bpv::environment::find_executable("sh"), {"-c", "exit 42"}));
#endif
}

