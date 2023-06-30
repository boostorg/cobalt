//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_PIPE_HPP
#define BOOST_ASYNC_IO_PIPE_HPP

#include <boost/async/io/stream.hpp>

#include <boost/asio/basic_readable_pipe.hpp>
#include <boost/asio/basic_writable_pipe.hpp>

namespace boost::async::io
{

system::result<std::pair<struct readable_pipe, struct writable_pipe>> make_pipe();

struct readable_pipe final : stream
{
  system::result<void> close() override;
  system::result<void> cancel() override;
  bool is_open() const override;

  using native_handle_type = typename asio::basic_readable_pipe<asio::io_context::executor_type>::native_handle_type;
  native_handle_type native_handle() {return pipe_.native_handle();}

  readable_pipe();
  readable_pipe(native_handle_type native_handle);

  system::result<void> assign(native_handle_type native_handle);
  system::result<native_handle_type> release();

  system::result<readable_pipe> duplicate();

  void write_some(buffers::mutable_buffer_span buffers) = delete;
  void write_some(buffers::mutable_buffer      buffer) = delete;

 private:
  void async_read_some_impl_(buffers::mutable_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  void async_write_some_impl_(buffers::const_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;

  friend system::result<std::pair<readable_pipe, writable_pipe>> make_pipe();
  asio::basic_readable_pipe<executor_type> pipe_;
};

struct writable_pipe final : stream
{
  system::result<void> close() override;
  system::result<void> cancel() override;
  bool is_open() const override;

  using native_handle_type = typename asio::basic_readable_pipe<asio::io_context::executor_type>::native_handle_type;
  native_handle_type native_handle() {return pipe_.native_handle();}

  writable_pipe();
  writable_pipe(native_handle_type native_handle);

  system::result<void> assign(native_handle_type native_handle);
  system::result<native_handle_type> release();
  system::result<writable_pipe> duplicate();

  void read_some(buffers::mutable_buffer_span buffers) = delete;
  void read_some(buffers::mutable_buffer      buffer) = delete;
 private:
  void async_read_some_impl_(buffers::mutable_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  void async_write_some_impl_(buffers::const_buffer_span buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  friend system::result<std::pair<readable_pipe, writable_pipe>> make_pipe();
  asio::basic_writable_pipe<executor_type> pipe_;
};

system::result<std::pair<readable_pipe, writable_pipe>> make_pipe();


}

#endif //BOOST_ASYNC_IO_PIPE_HPP
