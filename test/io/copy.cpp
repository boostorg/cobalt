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
#include <boost/async/io/copy.hpp>
#include <boost/async/io/read_all.hpp>
#include <boost/async/io/read.hpp>
#include <boost/async/io/write.hpp>
#include <boost/async/join.hpp>
#include <boost/asio.hpp>

#include <random>

using namespace boost::async;

promise<void> do_write__(io::stream & str, const std::string & input )
{
  auto w = co_await io::write(str, input);
  CHECK(w.transferred == input.size());
  str.close().value();
};

promise<void> do_copy(io::stream & in, io::stream & out)
{
  auto [r, w] = co_await io::copy(in, out);
  CHECK(r.transferred == (1024*1024));
  CHECK(w.value() == (1024*1024));
  out.close().value();

};

CO_TEST_CASE("copy")
{
  std::signal(SIGPIPE, SIG_IGN);
  auto [r, wi] = io::make_pipe().value();
  auto [ri, w] = io::make_pipe().value();

  std::string input;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> distPrintable(32,126);

  input.resize(1024*1024);
  std::generate(input.begin(), input.end(), [&]{return static_cast<char>(distPrintable(rng));});

  auto p = do_write__(w, input);
  auto c = do_copy(ri, wi);
  std::string output;
  auto res = co_await io::read_all(r, output);

  CHECK(res.transferred == output.size());
  CHECK(res.transferred == input.size());

  CHECK(std::equal(output.begin(), std::next(output.begin(), res.transferred), input.begin()));

  CHECK(!r.close().has_error());
  co_await p;
  co_await c;
}

