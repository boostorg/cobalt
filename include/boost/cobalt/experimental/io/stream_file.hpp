//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_STREAM_FILE_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_STREAM_FILE_HPP

#include <boost/asio/basic_stream_file.hpp>
#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/cobalt/config.hpp>
#include <boost/cobalt/experimental/io/buffer.hpp>
#include <boost/cobalt/experimental/io/ops.hpp>
#include <boost/cobalt/noop.hpp>
#include <boost/cobalt/op.hpp>

namespace boost::cobalt::experimental::io
{


struct stream_file : boost::asio::file_base
{
  using native_handle_type = asio::basic_stream_file<executor>::native_handle_type;

  BOOST_COBALT_DECL stream_file();
  BOOST_COBALT_DECL stream_file(const char * path, file_base::flags open_flags);
  BOOST_COBALT_DECL stream_file(const std::string & path, file_base::flags open_flags);
  BOOST_COBALT_DECL stream_file(const native_handle_type & native_file);
  BOOST_COBALT_DECL stream_file(stream_file && sf) noexcept;

  BOOST_COBALT_DECL system::result<void> assign(const native_handle_type & native_file);
  BOOST_COBALT_DECL system::result<void> cancel();

  BOOST_COBALT_DECL executor get_executor();
  BOOST_COBALT_DECL bool is_open() const;

  transfer_op auto write_some(const_buffer_sequence buffer)
  {
    return write_some_op_{implementation_, std::move(buffer)};
  }
  transfer_op auto read_some(mutable_buffer_sequence buffer)
  {
    return read_some_op_{implementation_, std::move(buffer)};
  }

  close_op auto close() {return close_op_{implementation_};}

  BOOST_COBALT_DECL native_handle_type native_handle();

  BOOST_COBALT_DECL system::result<void> open(const char * path,        flags open_flags);
  BOOST_COBALT_DECL system::result<void> open(const std::string & path, flags open_flags);

  BOOST_COBALT_DECL system::result<native_handle_type> release();
  BOOST_COBALT_DECL system::result<void> resize(std::uint64_t n);

  BOOST_COBALT_DECL system::result<std::uint64_t> seek(
      std::int64_t offset,
      seek_basis whence);

  BOOST_COBALT_DECL system::result<std::uint64_t> size() const;
  BOOST_COBALT_DECL system::result<void> sync_all();
  BOOST_COBALT_DECL system::result<void> sync_data();


 private:
  struct close_op_
  {
    asio::basic_stream_file<executor> & f_;
    constexpr bool await_ready() {return true;}
    constexpr void await_suspend(std::coroutine_handle<>) {}
    BOOST_COBALT_DECL           system::result<void> await_resume(as_result_tag);
    BOOST_COBALT_DECL std::tuple<system::error_code> await_resume(as_tuple_tag);
    BOOST_COBALT_DECL void                           await_resume();
  };

  struct read_some_op_ final : cobalt::op<system::error_code, std::size_t>
  {
    asio::basic_stream_file<executor> &implementation;
    mutable_buffer_sequence buffer;

    read_some_op_(asio::basic_stream_file<executor> & implementation, mutable_buffer_sequence buffer)
        : implementation(implementation), buffer(std::move(buffer)) {}

    BOOST_COBALT_DECL
    void initiate(cobalt::completion_handler<system::error_code, std::size_t> complete);
  };

  struct write_some_op_ final : cobalt::op<system::error_code, std::size_t>
  {
    asio::basic_stream_file<executor> &implementation;
    const_buffer_sequence buffer;

    write_some_op_(asio::basic_stream_file<executor> & implementation, const_buffer_sequence buffer)
      : implementation(implementation), buffer(std::move(buffer)) {}


    BOOST_COBALT_DECL
    void initiate(cobalt::completion_handler<system::error_code, std::size_t> complete);
  };


  asio::basic_stream_file<executor> implementation_;

};


}

#endif
#endif //BOOST_COBALT_EXPERIMENTAL_IO_STREAM_FILE_HPP
