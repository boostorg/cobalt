//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_PIPE_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_PIPE_HPP

#include <boost/cobalt/config.hpp>
#include <boost/cobalt/experimental/io/buffer.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>
#include <boost/cobalt/op.hpp>

#include <boost/asio/basic_readable_pipe.hpp>
#include <boost/asio/basic_writable_pipe.hpp>

#include <boost/system/result.hpp>

namespace boost::cobalt::experimental::io
{

BOOST_COBALT_DECL
system::result<std::pair<struct readable_pipe, struct  writable_pipe>> pipe();


struct readable_pipe 
{
  using native_handle_type = asio::basic_readable_pipe<executor>::native_handle_type;

  BOOST_COBALT_DECL readable_pipe();
  BOOST_COBALT_DECL readable_pipe(const native_handle_type & native_file);
  BOOST_COBALT_DECL readable_pipe(readable_pipe && sf) noexcept;

  BOOST_COBALT_DECL system::result<void> assign(const native_handle_type & native_file);
  BOOST_COBALT_DECL system::result<void> cancel();

  BOOST_COBALT_DECL executor get_executor();
  BOOST_COBALT_DECL bool is_open() const;

  BOOST_COBALT_DECL system::result<void> close();

  BOOST_COBALT_DECL native_handle_type native_handle();
  BOOST_COBALT_DECL system::result<native_handle_type> release();

  read_op read_some(mutable_buffer_sequence buffer)
  {
    return {buffer, this, initiate_read_some_};
  }

 private:

  BOOST_COBALT_DECL static void initiate_read_some_(void *, mutable_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  friend system::result<std::pair<struct readable_pipe, struct writable_pipe>> pipe();
  asio::basic_readable_pipe<executor> implementation_;
};


struct writable_pipe
{
  using native_handle_type = asio::basic_writable_pipe<executor>::native_handle_type;

  BOOST_COBALT_DECL writable_pipe();
  BOOST_COBALT_DECL writable_pipe(const native_handle_type & native_file);
  BOOST_COBALT_DECL writable_pipe(writable_pipe && sf) noexcept;

  BOOST_COBALT_DECL system::result<void> assign(const native_handle_type & native_file);
  BOOST_COBALT_DECL system::result<void> cancel();

  BOOST_COBALT_DECL executor get_executor();
  BOOST_COBALT_DECL bool is_open() const;

  write_op write_some(const_buffer_sequence buffer)
  {
    return {buffer, this, initiate_write_some_};
  }

  BOOST_COBALT_DECL system::result<void> close();

  BOOST_COBALT_DECL native_handle_type native_handle();

  BOOST_COBALT_DECL system::result<native_handle_type> release();

 private:
  BOOST_COBALT_DECL static void initiate_write_some_(void *, const_buffer_sequence, boost::cobalt::completion_handler<boost::system::error_code, std::size_t>);

  friend system::result<std::pair<struct readable_pipe, struct  writable_pipe>> pipe();
  asio::basic_writable_pipe<executor> implementation_;
};



}

#endif //BOOST_COBALT_EXPERIMENTAL_IO_PIPE_HPP
