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

system::result<std::pair<struct readable_pipe, struct writable_pipe>> make_pipe(
    const async::executor & executor = this_thread::get_executor()
    );

struct readable_pipe final : stream
{
  BOOST_ASYNC_DECL system::result<void> close() override;
  BOOST_ASYNC_DECL system::result<void> cancel() override;
  BOOST_ASYNC_DECL bool is_open() const override;

  using native_handle_type = typename asio::basic_readable_pipe<executor>::native_handle_type;
  native_handle_type native_handle() {return pipe_.native_handle();}

  BOOST_ASYNC_DECL readable_pipe(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL readable_pipe(native_handle_type native_handle, const async::executor & executor = this_thread::get_executor());

  BOOST_ASYNC_DECL system::result<void> assign(native_handle_type native_handle);
  BOOST_ASYNC_DECL system::result<native_handle_type> release();

  BOOST_ASYNC_DECL system::result<readable_pipe> duplicate(const async::executor & executor = this_thread::get_executor());

  void write_some(buffers::mutable_buffer_subspan buffers) = delete;
  void write_some(buffers::mutable_buffer      buffer) = delete;

 private:
  BOOST_ASYNC_DECL void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;

  friend system::result<std::pair<readable_pipe, writable_pipe>> make_pipe(const async::executor & exec);
  asio::basic_readable_pipe<executor> pipe_;
};

struct writable_pipe final : stream
{
  BOOST_ASYNC_DECL system::result<void> close() override;
  BOOST_ASYNC_DECL system::result<void> cancel() override;
  BOOST_ASYNC_DECL bool is_open() const override;

  using native_handle_type = typename asio::basic_readable_pipe<executor>::native_handle_type;
  native_handle_type native_handle() {return pipe_.native_handle();}

  BOOST_ASYNC_DECL writable_pipe(const async::executor & executor = this_thread::get_executor());
  BOOST_ASYNC_DECL writable_pipe(native_handle_type native_handle,
                                 const async::executor & executor = this_thread::get_executor());

  BOOST_ASYNC_DECL system::result<void> assign(native_handle_type native_handle);
  BOOST_ASYNC_DECL system::result<native_handle_type> release();
  BOOST_ASYNC_DECL system::result<writable_pipe> duplicate(const async::executor & executor = this_thread::get_executor());

  void read_some(buffers::mutable_buffer_subspan buffers) = delete;
  void read_some(buffers::mutable_buffer      buffer) = delete;
 private:
  BOOST_ASYNC_DECL void async_read_some_impl_(buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_impl_(buffers::const_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  friend system::result<std::pair<readable_pipe, writable_pipe>> make_pipe(const async::executor & exec);
  asio::basic_writable_pipe<executor> pipe_;
};


}

#endif //BOOST_ASYNC_IO_PIPE_HPP
