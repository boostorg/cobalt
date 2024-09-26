//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_STREAM_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_STREAM_HPP

#include <boost/cobalt/experimental/io/buffer.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>

#include <boost/asio/execution/executor.hpp>

namespace boost::cobalt::experimental::io
{

template<typename Stream>
concept read_stream =
    requires (Stream & str, mutable_buffer_sequence buf) {
      {str.get_executor()} -> asio::execution::executor;
      {str.read_some(buf)} -> transfer_op;
    };

template<typename Stream>
concept write_stream =
    requires (Stream & str, const_buffer_sequence buf) {
      {str.get_executor() } -> asio::execution::executor;
      {str.write_some(buf)} -> transfer_op;
    };

template<typename Stream>
concept stream = read_stream<Stream> && write_stream<Stream>;


template<typename Stream>
concept random_access_read_device =
    requires (Stream & str, std::size_t o, mutable_buffer_sequence buf) {
      { str.get_executor() } -> asio::execution::executor;
      { str.read_some_at(o, buf) } -> transfer_op;
    };

template<typename Stream>
concept random_access_write_device =
    requires (Stream & str, std::size_t o, const_buffer_sequence buf) {
      { str.get_executor() }        -> asio::execution::executor;
      { str.write_some_at(o, buf) } -> transfer_op;
    };

template<typename Stream>
concept random_access_device = random_access_read_device<Stream> && random_access_write_device<Stream>;

}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_STREAM_HPP
