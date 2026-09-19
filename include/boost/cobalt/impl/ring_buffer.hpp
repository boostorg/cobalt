//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_IMPL_RING_BUFFER_HPP
#define BOOST_COBALT_IMPL_RING_BUFFER_HPP

#include <boost/cobalt/ring_buffer.hpp>
#include <boost/cobalt/result.hpp>

#include <boost/asio/post.hpp>

namespace boost::cobalt
{
#if !defined(BOOST_COBALT_NO_PMR)
template<typename T>
inline ring_buffer<T>::ring_buffer(
    std::size_t limit,
    executor executor,
    pmr::memory_resource * resource)
    : buffer_(limit, pmr::polymorphic_allocator<T>(resource)), executor_(executor) {}
#else
template<typename T>
inline ring_buffer<T>::ring_buffer(
    std::size_t limit,
    executor executor)
    : buffer_(limit), executor_(executor) {}
#endif

template<typename T>
auto ring_buffer<T>::get_executor() -> const executor_type &  {return executor_;}

template<typename T>
bool ring_buffer<T>::is_open() const {return !is_closed_;}

template<typename T>
ring_buffer<T>::~ring_buffer()
{
  while (!read_queue_.empty())
    read_queue_.front().awaited_from.reset();
}

template<typename T>
void ring_buffer<T>::close()
{
  is_closed_ = true;
  while (!read_queue_.empty())
  {
    auto & op = read_queue_.front();
    op.unlink();
    op.cancelled = true;
    op.cancel_slot.clear();

    if (op.awaited_from)
      asio::post(executor_, std::move(op.awaited_from));
  }
}

template<typename T>
std::size_t ring_buffer<T>::available() const {return buffer_.size();}

template<typename T>
struct ring_buffer<T>::read_op::cancel_impl
{
  read_op * op;
  cancel_impl(read_op * op) : op(op) {}
  void operator()(asio::cancellation_type)
  {
    op->cancelled = true;
    op->unlink();
    if (op->awaited_from)
      asio::post(
          op->buf->executor_,
          std::move(op->awaited_from));
    op->cancel_slot.clear();
  }
};

template<typename T>
template<typename Promise>
std::coroutine_handle<void> ring_buffer<T>::read_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if (cancelled)
    return h; // already interrupted.

  if constexpr (requires {h.promise().get_cancellation_slot();})
    if ((cancel_slot = h.promise().get_cancellation_slot()).is_connected())
      cancel_slot.emplace<cancel_impl>(this);

  if (awaited_from)
    boost::throw_exception(std::runtime_error("already-awaited"), loc);
  awaited_from.reset(h.address());
  // currently nothing to read
  if constexpr (requires {h.promise().begin_transaction();})
    begin_transaction = +[](void * p){std::coroutine_handle<Promise>::from_address(p).promise().begin_transaction();};

  buf->read_queue_.push_back(*this);
  return std::noop_coroutine();
}


template<typename T>
T ring_buffer<T>::read_op::await_resume()
{
  return await_resume(as_result_tag{}).value(loc);
}

template<typename T>
std::tuple<system::error_code, T> ring_buffer<T>::read_op::await_resume(const struct as_tuple_tag &)
{
  auto res = await_resume(as_result_tag{});

  if (res.has_error())
    return {res.error(), T{}};
  else
    return {system::error_code{}, std::move(*res)};

}

template<typename T>
system::result<T> ring_buffer<T>::read_op::await_resume(const struct as_result_tag &)
{
  if (cancel_slot.is_connected())
    cancel_slot.clear();

  if (direct)
    return std::move(*direct);

  if (buf->is_closed_ && buf->buffer_.empty())
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return {system::in_place_error, asio::error::broken_pipe, &loc};
  }

  if (cancelled)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return {system::in_place_error, asio::error::operation_aborted, &loc};
  }

  auto value = std::move(buf->buffer_.front());
  buf->buffer_.pop_front();
  return {system::in_place_value, std::move(value)};
}

template<typename T>
template<typename Promise>
std::coroutine_handle<void> ring_buffer<T>::write_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if (cancelled)
    return h;

  if (direct = !buf->read_queue_.empty())
  {
    auto & op = buf->read_queue_.front();

    if (buf->buffer_.empty())
    {
      op.direct = std::move(value);
    }
    else
    {
      op.direct = std::move(buf->buffer_.front());
      buf->buffer_.pop_front();
      buf->buffer_.push_back(std::move(value));
    }

    op.transactional_unlink();
    asio::post(buf->executor_, unique_handle<Promise>::from_promise(h.promise()));
    return op.awaited_from.release();
  }

  return h;
}

template<typename T>
void ring_buffer<T>::post(T && value)
{
  if (!read_queue_.empty())
  {
    auto & op = read_queue_.front();

    if (buffer_.empty())
    {
      op.direct = std::move(value);
    }
    else
    {
      op.direct = std::move(buffer_.front());
      buffer_.pop_front();
      buffer_.push_back(std::move(value));
    }

    op.transactional_unlink();
    asio::post(executor_, std::move(op.awaited_from));
  }
  else if (buffer_.capacity() > 0u)
    buffer_.push_back(std::move(value));
}


template<typename T>
std::tuple<system::error_code> ring_buffer<T>::write_op::await_resume(const struct as_tuple_tag &)
{
  return await_resume(as_result_tag{}).error();
}

template<typename T>
void ring_buffer<T>::write_op::await_resume()
{
  await_resume(as_result_tag{}).value(loc);
}

template<typename T>
system::result<void>  ring_buffer<T>::write_op::await_resume(const struct as_result_tag &)
{
  if (buf->is_closed_)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return {system::in_place_error, asio::error::broken_pipe, &loc};
  }

  if (cancelled)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return {system::in_place_error, asio::error::operation_aborted, &loc};
  }
  if (!direct  && buf->buffer_.capacity() > 0u)
    buf->buffer_.push_back(std::move(value));

  return system::in_place_value;
}

struct ring_buffer<void>::read_op::cancel_impl
{
  read_op * op;
  cancel_impl(read_op * op) : op(op) {}
  void operator()(asio::cancellation_type)
  {
    op->cancelled = true;
    op->unlink();
    if (op->awaited_from)
      asio::post(op->buf->executor_, std::move(op->awaited_from));
    op->cancel_slot.clear();
  }
};

template<typename Promise>
std::coroutine_handle<void> ring_buffer<void>::read_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if (cancelled)
    return h; // already interrupted.

  if constexpr (requires {h.promise().get_cancellation_slot();})
    if ((cancel_slot = h.promise().get_cancellation_slot()).is_connected())
      cancel_slot.emplace<cancel_impl>(this);

  if (awaited_from)
    boost::throw_exception(std::runtime_error("already-awaited"), loc);
  awaited_from.reset(h.address());

  if constexpr (requires {h.promise().begin_transaction();})
    begin_transaction = +[](void * p){std::coroutine_handle<Promise>::from_address(p).promise().begin_transaction();};

  buf->read_queue_.push_back(*this);
  return std::noop_coroutine();
}


template<typename Promise>
std::coroutine_handle<void> ring_buffer<void>::write_op::await_suspend(std::coroutine_handle<Promise> h)
{
  if (cancelled)
    return h; // already interrupted.

  if (direct = !buf->read_queue_.empty())
  {
    auto & op = buf->read_queue_.front();

    op.direct = true;

    op.transactional_unlink();
    asio::post(buf->executor_, unique_handle<Promise>::from_promise(h.promise()));
    return op.awaited_from.release();
  }

  return h;
}

}

#endif //BOOST_COBALT_IMPL_RING_BUFFER_HPP
