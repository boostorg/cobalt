//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../doctest.h"
#include "../test.hpp"
#include <boost/async/io/pipe.hpp>
#include <boost/async/io/buffers.hpp>
#include <boost/async/io/read_until.hpp>
#include <boost/async/io/write.hpp>
#include <boost/async/join.hpp>
#include <boost/asio.hpp>

#include <random>

using namespace boost::async;

promise<void> do_write_(io::stream & str, std::string & input )
{
  boost::ignore_unused(co_await io::write(str, {input.data(), input.size()}));
};


CO_TEST_CASE("read_until char")
{
    auto [r, w] = io::make_pipe().value();

    std::string input;

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distPrintable(32,126);

    input.resize(1024*1024);
    std::generate(input.begin(), input.end(), [&]{return static_cast<char>(distPrintable(rng));});
    input[4242] = '\n';

    auto p = do_write_(w, input);


    std::string output;
    auto res = co_await io::read_until(r, output, '\n');

    CHECK(res.transferred >= 4242);
    CHECK(res.transferred <  10000);
    CHECK(!res.has_error());

    CHECK(std::equal(output.begin(), output.end(), input.begin()));

    CHECK(!r.close().has_error());
    co_await p;
}


CO_TEST_CASE("read_until string")
{
  auto [r, w] = io::make_pipe().value();

  std::string input;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> distPrintable(32,126);

  input.resize(1024*1024);
  std::generate(input.begin(), input.end(), [&]{return static_cast<char>(distPrintable(rng));});
  input[4242] = '\n';
  input[8000] = '\r';
  input[8001] = '\n';

  auto p = do_write_(w, input);


  std::string output;
  auto res = co_await io::read_until(r, output, "\r\n");

  CHECK(res.transferred >= 8001);
  CHECK(res.transferred <  16000);
  CHECK(!res.has_error());

  CHECK(std::equal(output.begin(), output.end(), input.begin()));

  CHECK(!r.close().has_error());
  co_await p;
}