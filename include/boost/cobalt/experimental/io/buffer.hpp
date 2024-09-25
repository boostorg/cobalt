//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP
#define BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP

#include <boost/asio/buffer.hpp>
#include <boost/range/any_range.hpp>
#include <boost/variant2/variant.hpp>

#include <span>

namespace boost::cobalt::experimental::io
{


// this is about 10-20% slower than a span
struct any_const_buffer_sequence
    : any_range<asio::const_buffer, boost::forward_traversal_tag>
{
  template<typename ConstBufferSequence>
  explicit (std::constructible_from<std::span<asio::const_buffer>, ConstBufferSequence>)
  any_const_buffer_sequence(ConstBufferSequence && buffer)
      : boost::any_range<asio::const_buffer, boost::forward_traversal_tag>(
      boost::asio::buffer_sequence_begin(buffer),
      boost::asio::buffer_sequence_end(buffer))
  {
  }
};

using const_buffer_sequence
    = boost::variant2::variant<
    asio::const_buffer,
    asio::const_registered_buffer,
    std::span<asio::const_buffer>,
any_const_buffer_sequence>;


// this is about 10-20% slower than a span
struct any_mutable_buffer_sequence
    : any_range<asio::mutable_buffer, boost::forward_traversal_tag>
{
  template<typename ConstBufferSequence>
  explicit (std::constructible_from<std::span<asio::mutable_buffer>, ConstBufferSequence>)
  any_mutable_buffer_sequence(ConstBufferSequence && buffer)
      : boost::any_range<asio::mutable_buffer, boost::forward_traversal_tag>(
      boost::asio::buffer_sequence_begin(buffer),
      boost::asio::buffer_sequence_end(buffer))
  {
  }
};

using mutable_buffer_sequence
    = boost::variant2::variant<
    asio::mutable_buffer,
    asio::mutable_registered_buffer,
    std::span<asio::mutable_buffer>,
any_mutable_buffer_sequence>;


}

#endif //BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP
