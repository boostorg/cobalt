//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_NOTHROW_HPP
#define BOOST_COBALT_NOTHROW_HPP

#include <boost/cobalt/config.hpp>

#include <boost/system/result.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/errc.hpp>

#include <coroutine>

namespace boost::cobalt
{

template<typename Handle>
struct nothrow : boost::system::result<Handle>
{
  nothrow(Handle && h) : boost::system::result<Handle>(boost::system::in_place_value, std::move(h)) {}
  nothrow() : boost::system::result<Handle>(boost::system::in_place_error,
                                            make_error_code(boost::system::errc::not_enough_memory))
  {}
};

}

template<typename Handle, typename ... Args>
struct std::coroutine_traits<boost::cobalt::nothrow<Handle>, Args...>
{
  struct promise_type : std::coroutine_traits<Handle, Args...>::promise_type
  {
    static void* operator new(std::size_t sz) noexcept
    {
      return ::operator new(sz, std::nothrow);
    }

    static void operator delete(void * raw, const std::size_t size) noexcept
    {
#if defined(__cpp_sized_deallocation)
      ::operator delete(raw, size);
#else
      ::operator delete(raw);
#endif
    }

    boost::cobalt::nothrow<Handle> get_return_object()
    {
      return std::coroutine_traits<Handle, Args...>::promise_type::get_return_object();
    }

    static boost::cobalt::nothrow<Handle> get_return_object_on_allocation_failure()
    {
      return {};
    }

  };
};

#endif //BOOST_COBALT_NOTHROW_HPP
