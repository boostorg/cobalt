//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/continuation.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/generator.hpp>

#include "../test.hpp"

using namespace boost::cobalt;

BOOST_AUTO_TEST_SUITE(continuation_);

struct awaitable
{
  bool await_ready() { return ready;}

  bool await_suspend(std::coroutine_handle<void> h) { this->h = h; return suspend;}

  void await_resume() {}

  std::coroutine_handle<void> h;
  bool ready{false};
  bool suspend{true};
};

awaitable aw;


boost::context::continuation test(boost::context::continuation && f)
{
  if (!aw.await_ready())
  {
    boost::cobalt::experimental::continuation_frame ff;
    f = std::move(f).resume_with(
        [&](boost::context::continuation && f_) -> boost::context::continuation
        {
          ff.continuation() = std::move(f_);
          std::coroutine_handle<boost::context::continuation> h(ff);

          if (!aw.await_suspend(h))
            return std::move(ff.continuation());

          return {};
        });

  }
  aw.await_resume();

  return f;
}

BOOST_AUTO_TEST_CASE(continuation_1)
{
  BOOST_CHECK(!boost::context::callcc(test));

  BOOST_CHECK(aw.h);
  aw.h.resume();
}


BOOST_AUTO_TEST_CASE(continuation_2)
{
  aw.suspend = false;
  aw.h = {};
  BOOST_CHECK(boost::context::callcc(test));
  BOOST_CHECK(aw.h);
}

BOOST_AUTO_TEST_CASE(continuation_3)
{
  aw.ready = true;
  aw.suspend = false;
  aw.h = {};
  BOOST_CHECK(!boost::context::callcc(test));

  BOOST_CHECK(!aw.h);
}



BOOST_AUTO_TEST_SUITE_END();
