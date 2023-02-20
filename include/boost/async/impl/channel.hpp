//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IMPL_CHANNEL_HPP
#define BOOST_ASYNC_IMPL_CHANNEL_HPP

#include <boost/async/channel.hpp>

namespace boost::async
{


template<typename T>
channel<T>::~channel()
{
  while (!read_queue_.empty())
    read_queue_.front().awaited_from.reset();

  while (!write_queue_.empty())
    write_queue_.front().awaited_from.reset();

}

template<typename T>
void channel<T>::close()
{
  is_closed_ = true;
  while (!read_queue_.empty())
  {
    auto & op = read_queue_.front();
    op.unlink();
    op.cancelled = true;
    op.cancel_slot.clear();
    asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
  }
  while (!write_queue_.empty())
  {
    auto & op = write_queue_.front();
    op.unlink();
    op.cancelled = true;
    op.cancel_slot.clear();
    asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
  }
}


template<typename T>
struct  channel<T>::read_op::cancel_impl
{
  read_op * op;
  cancel_impl(read_op * op) : op(op) {}
  void operator()(asio::cancellation_type ct)
  {
    op->cancelled = true;
    op->unlink();
    asio::post(
        op->chn->executor_,
        [h = std::move(op->awaited_from)]() mutable
        {
          std::coroutine_handle<void>::from_address(h.release()).resume();
        });
  }
};

template<typename T>
template<typename Promise>
std::coroutine_handle<void> channel<T>::read_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if constexpr (requires (Promise p) {p.get_cancellation_slot();})
  if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
    (cancel_slot = sl).template emplace<cancel_impl>(this);

  awaited_from.reset(h.address());
  // currently nothing to read
  chn->read_queue_.push_back(*this);

  if (chn->write_queue_.empty())
    return std::noop_coroutine();
  else
  {
    auto & op = chn->write_queue_.front();
    op.unlink();
    BOOST_ASSERT(op.awaited_from);
    return std::coroutine_handle<void>::from_address(op.awaited_from.release());
  }
}

template<typename T>
T channel<T>::read_op::await_resume()
{
  if (cancel_slot.is_connected())
    cancel_slot.clear();

  if (cancelled)
    boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
  else
  {
    if (!chn->write_queue_.empty())
    {
      auto & op = chn->write_queue_.front();
      op.unlink();
      BOOST_ASSERT(op.awaited_from);
      asio::post(
          chn->executor_,
          [h = std::move(op.awaited_from)]() mutable
          {
            std::coroutine_handle<void>::from_address(h.release()).resume();
          });
    }
    T value = std::move(chn->buffer_.front());
    chn->buffer_.pop_front();
    return value;
  }
}

template<typename T>
struct channel<T>::write_op::cancel_impl
{
  write_op * op;
  cancel_impl(write_op * op) : op(op) {}
  void operator()(asio::cancellation_type ct)
  {
    op->cancelled = true;
    op->unlink();
    asio::post(
        op->chn->executor_,
        [h = std::move(op->awaited_from)]() mutable
        {
          std::coroutine_handle<void>::from_address(h.release()).resume();
        });
  }
};

template<typename T>
template<typename Promise>
std::coroutine_handle<void> channel<T>::write_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if constexpr (requires (Promise p) {p.get_cancellation_slot();})
    if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
      (cancel_slot = sl).template emplace<cancel_impl>(this);

  awaited_from.reset(h.address());
  // currently nothing to read
  chn->write_queue_.push_back(*this);

  if (chn->read_queue_.empty())
    return std::noop_coroutine();
  else
  {
    auto & op = chn->read_queue_.front();
    op.unlink();
    BOOST_ASSERT(op.awaited_from);
    return std::coroutine_handle<void>::from_address(op.awaited_from.release());
  }
}

template<typename T>
void channel<T>::write_op::await_resume()
{
  if (cancel_slot.is_connected())
    cancel_slot.clear();
  if (cancelled)
    boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
  else
  {
    if (!chn->read_queue_.empty())
    {
      auto & op = chn->read_queue_.front();
      op.unlink();
      BOOST_ASSERT(op.awaited_from);
      asio::post(
          chn->executor_,
          [h = std::move(op.awaited_from)]() mutable
          {
            std::coroutine_handle<void>::from_address(h.release()).resume();
          });
    }
    if (ref.index() == 0)
      chn->buffer_.push_back(std::move(*variant2::get<0>(ref)));
    else
      chn->buffer_.push_back(*variant2::get<1>(ref));
  }
}

struct channel<void>::read_op::cancel_impl
{
  read_op * op;
  cancel_impl(read_op * op) : op(op) {}
  void operator()(asio::cancellation_type ct)
  {
    op->cancelled = true;
    op->unlink();
    asio::post(
        op->chn->executor_,
        [h = std::move(op->awaited_from)]() mutable
        {
          std::coroutine_handle<void>::from_address(h.release()).resume();
        });
  }
};

template<typename Promise>
std::coroutine_handle<void> channel<void>::read_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if constexpr (requires (Promise p) {p.get_cancellation_slot();})
    if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
      (cancel_slot = sl).template emplace<cancel_impl>(this);

  awaited_from.reset(h.address());
  // currently nothing to read
  chn->read_queue_.push_back(*this);

  if (chn->write_queue_.empty())
    return std::noop_coroutine();
  else
  {
    auto & op = chn->write_queue_.front();
    op.unlink();
    BOOST_ASSERT(op.awaited_from);
    return std::coroutine_handle<void>::from_address(op.awaited_from.release());
  }
}

struct channel<void>::write_op::cancel_impl
{
  write_op * op;
  cancel_impl(write_op * op) : op(op) {}
  void operator()(asio::cancellation_type ct)
  {
    op->cancelled = true;
    op->unlink();
    asio::post(
        op->chn->executor_,
        [h = std::move(op->awaited_from)]() mutable
        {
          std::coroutine_handle<void>::from_address(h.release()).resume();
        });
  }
};

template<typename Promise>
std::coroutine_handle<void> channel<void>::write_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if constexpr (requires (Promise p) {p.get_cancellation_slot();})
    if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
      (cancel_slot = sl).template emplace<cancel_impl>(this);

  awaited_from.reset(h.address());
  // currently nothing to read
  chn->write_queue_.push_back(*this);

  if (chn->read_queue_.empty())
    return std::noop_coroutine();
  else
  {
    auto & op = chn->read_queue_.front();
    op.unlink();
    BOOST_ASSERT(op.awaited_from);
    return std::coroutine_handle<void>::from_address(op.awaited_from.release());
  }
}

}

#endif //BOOST_ASYNC_IMPL_CHANNEL_HPP
