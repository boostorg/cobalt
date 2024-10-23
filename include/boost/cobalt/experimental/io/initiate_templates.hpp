//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_OP_IMPLEMENTATION_TEMPLATES_HPP
#define BOOST_COBALT_EXPERIMENTAL_OP_IMPLEMENTATION_TEMPLATES_HPP

#include <boost/cobalt/experimental/io/buffer.hpp>
#include <boost/cobalt/detail/handler.hpp>

namespace boost::cobalt::experimental::io
{

template<typename Stream>
void initiate_async_read_some(Stream & str, mutable_buffer_sequence seq,
                              completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_read_some(seq, std::move(handler));
  else if (seq.is_registered())
    str.async_read_some(seq.registered, std::move(handler));
  else
    str.async_read_some(seq.head, std::move(handler));
}

template<typename Stream>
void initiate_async_write_some(Stream & str, const_buffer_sequence seq,
                               completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_write_some(seq, std::move(handler));
  else if (seq.is_registered())
    str.async_write_some(seq.registered, std::move(handler));
  else
    str.async_write_some(seq.head, std::move(handler));
}

template<typename Stream>
void initiate_async_read_some_at(Stream & str, std::uint64_t offset, mutable_buffer_sequence seq,
                              completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_read_some_at(offset, seq, std::move(handler));
  else if (seq.is_registered())
    str.async_read_some_at(offset, seq.registered, std::move(handler));
  else
    str.async_read_some_at(offset, seq.head, std::move(handler));
}

template<typename Stream>
void initiate_async_write_some_at(Stream & str, std::uint64_t offset, const_buffer_sequence seq,
                               completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_write_some_at(offset, seq, std::move(handler));
  else if (seq.is_registered())
    str.async_write_some_at(offset, seq.registered, std::move(handler));
  else
    str.async_write_some_at(offset, seq.head, std::move(handler));
}

template<typename Stream>
void initiate_async_send(Stream & str, const_buffer_sequence seq,
                         completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_send(seq, std::move(handler));
  else if (seq.is_registered())
    str.async_send(seq.registered, std::move(handler));
  else
    str.async_send(seq.head, std::move(handler));
}

template<typename Stream>
void initiate_async_receive(Stream & str, mutable_buffer_sequence seq,
                            completion_handler<boost::system::error_code, std::size_t> handler)
{
  if (seq.buffer_count() > 0u)
    str.async_receive(seq, std::move(handler));
  else if (seq.is_registered())
    str.async_receive(seq.registered, std::move(handler));
  else
    str.async_receive(seq.head, std::move(handler));
}

}

#endif //BOOST_COBALT_EXPERIMENTAL_OP_IMPLEMENTATION_TEMPLATES_HPP
