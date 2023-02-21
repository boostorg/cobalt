//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_WAIT_GROUP_HPP
#define BOOST_ASYNC_DETAIL_WAIT_GROUP_HPP

#include <boost/async/promise.hpp>
#include <boost/async/select.hpp>
#include <boost/async/wait.hpp>

#include <list>


namespace boost::async::detail
{

struct select_wrapper
{
  using impl_type = decltype(select(std::declval<std::list<promise<void>> &>()));
  std::list<promise<void>> &waitables_;
  std::optional<impl_type> impl_;

  select_wrapper(std::list<promise<void>> &waitables) : waitables_(waitables)
  {
    if (!waitables_.empty())
      impl_.emplace(waitables_);

  }

  bool await_ready()
  {
    if (waitables_.empty())
      return true;
    else
      return impl_->await_ready();
  }

  template<typename Promise>
  auto await_suspend(std::coroutine_handle<Promise> h)
  {
    return impl_->await_suspend(h);
  }

  void await_resume()
  {
    if (waitables_.empty())
      return;
    auto idx = impl_->await_resume();
    if (idx != -1)
      waitables_.erase(std::next(waitables_.begin(), idx));
  }
};

struct wait_wrapper
{
  using impl_type = decltype(wait(std::declval<std::list<promise<void>> &>()));
  std::list<promise<void>> &waitables_;
  std::optional<impl_type> impl_;

  wait_wrapper(std::list<promise<void>> &waitables) : waitables_(waitables)
  {
    if (!waitables_.empty())
      impl_.emplace(wait(waitables_));

  }

  bool await_ready()
  {
    if (waitables_.empty())
      return true;
    else
      return impl_->await_ready();
  }

  template<typename Promise>
  auto await_suspend(std::coroutine_handle<Promise> h)
  {
    return impl_->await_suspend(h);
  }

  void await_resume()
  {
    if (waitables_.empty())
      return;
    BOOST_ASSERT(impl_);
    waitables_.clear();
    impl_->await_resume();
  }
};

}

#endif //BOOST_ASYNC_DETAIL_WAIT_GROUP_HPP
