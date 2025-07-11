//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/ring_buffer.hpp>
#include <boost/asio/defer.hpp>

namespace boost::cobalt
{

ring_buffer<void>::~ring_buffer()
{
  while (!read_queue_.empty())
    read_queue_.front().awaited_from.reset();
}

void ring_buffer<void>::close()
{
  is_closed_ = true;
  while (!read_queue_.empty()) {
    auto &op = read_queue_.front();
    op.unlink();
    op.cancelled = true;
    op.cancel_slot.clear();
    if (op.awaited_from)
      asio::defer(executor_, std::move(op.awaited_from));
  }
}

system::result<void>  ring_buffer<void>::read_op::await_resume(const struct as_result_tag &)
{
  if (cancel_slot.is_connected())
    cancel_slot.clear();
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

  if (!direct)
    buf->n_--;
  return {system::in_place_value};
}


void ring_buffer<void>::read_op::await_resume()
{
  await_resume(as_result_tag{}).value(loc);
}

std::tuple<system::error_code> ring_buffer<void>::read_op::await_resume(const struct as_tuple_tag & )
{
  return await_resume(as_result_tag{}).error();
}

void ring_buffer<void>::write_op::await_resume()
{
  await_resume(as_result_tag{}).value(loc);
}

std::tuple<system::error_code> ring_buffer<void>::write_op::await_resume(const struct as_tuple_tag &)
{
  return await_resume(as_result_tag{}).error();
}

system::result<void> ring_buffer<void>::write_op::await_resume(const as_result_tag &)
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

  if (!direct && buf->n_ < buf->limit_)
    buf->n_++;

  return system::in_place_value;
}

void ring_buffer<void>::post()
{
  if (!read_queue_.empty())
  {
    auto & op = read_queue_.front();
    if (n_ == limit_)
      op.direct = true;
    op.transactional_unlink();
    asio::defer(executor_, std::move(op.awaited_from));
  }
  else if (n_ < limit_)
    n_++;
}


}
