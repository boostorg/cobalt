//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_ANY_HPP
#define BOOST_ASYNC_ANY_HPP

#include <boost/async/config.hpp>
#include <boost/async/this_thread.hpp>

#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/cancellation_signal.hpp>

#include <coroutine>

namespace boost::async
{

namespace detail
{

struct any_coroutine_handle_vtable_t
{
  asio::cancellation_slot          (*get_cancellation_slot)(void*);
  executor                         (*get_executor)(void*);
  pmr::polymorphic_allocator<void> (*get_allocator)(void*);
};

template<typename Promise>
constexpr static any_coroutine_handle_vtable_t any_coroutine_handle_vtable {
  .get_cancellation_slot=
      +[](void * p)
      {
        auto h = std::coroutine_handle<Promise>::from_address(p);
        return asio::associated_cancellation_slot(h.promise());
      },
  .get_executor=
    +[](void * p)
    {
      auto h = std::coroutine_handle<Promise>::from_address(p);
      return asio::associated_executor(h.promise(), this_thread::get_executor());
    },
  .get_allocator=
    +[](void * p)
    {
      auto h = std::coroutine_handle<Promise>::from_address(p);
      return asio::associated_allocator(h.promise(), this_thread::get_allocator());
    }
};

}

struct any_coroutine_handle
{
  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const noexcept
  {
    return vtable_.get_cancellation_slot(handle.address());
  }

  using executor_type = executor;
  const executor_type & get_executor() const noexcept
  {
    return vtable_.get_executor(handle.address());
  }

  using allocator_type = pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() const noexcept
  {
    return vtable_.get_allocator(handle.address());
  }


  template<typename Promise>
  any_coroutine_handle(std::coroutine_handle<Promise> p)
    : handle(p), vtable_(detail::any_coroutine_handle_vtable<Promise>) {}

  std::coroutine_handle<void> handle;
 private:
  const detail::any_coroutine_handle_vtable_t & vtable_;
};




}

#endif //BOOST_ASYNC_ANY_HPP
