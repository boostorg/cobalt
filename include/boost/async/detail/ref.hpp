// Copyright (c) 2023 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_DETAIL_ASYNC_REF_HPP
#define BOOST_DETAIL_ASYNC_REF_HPP

#include <coroutine>

namespace boost::async::detail
{

template<typename T>
struct reference_wrapper
{
  T & impl;

  bool await_ready() noexcept (noexcept(impl.await_ready()))
  {
    return impl.await_ready();
  }

  template<typename Promise>
  auto await_suspend(std::coroutine_handle<Promise> h) noexcept (noexcept(impl.await_suspend(h)))
      -> decltype(impl.await_suspend(h))
  {
    return impl.await_suspend(h);
  }

  auto await_resume() noexcept(noexcept(impl.await_resume()))
      -> decltype(impl.await_resume())
  {
    return impl.await_resume();
  }

};

template<typename T>
reference_wrapper<T> ref(T & t)
{
  return reference_wrapper<T>{t};
}


}

#endif //BOOST_DETAIL_ASYNC_REF_HPP
