//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_RANDOM_ACCESS_FILE_HPP
#define BOOST_ASYNC_RANDOM_ACCESS_FILE_HPP

#include <boost/async/io/file.hpp>
#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/async/io/random_access_device.hpp>
#include <boost/asio/basic_random_access_file.hpp>

namespace boost::async::io
{

struct random_access_file : file, random_access_device
{
  BOOST_ASYNC_DECL system::result<random_access_file> duplicate();

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> close() override;
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> cancel() override;
  [[nodiscard]] BOOST_ASYNC_DECL bool is_open() const override;

  BOOST_ASYNC_DECL random_access_file();
  BOOST_ASYNC_DECL random_access_file(random_access_file && lhs);
  BOOST_ASYNC_DECL random_access_file(native_handle_type h);
  BOOST_ASYNC_DECL random_access_file(core::string_view file, flags open_flags = flags::read_write);
 private:
  BOOST_ASYNC_DECL void async_read_some_at_impl_ (std::uint64_t offset, buffers::mutable_buffer_subspan buffer, async::completion_handler<system::error_code, std::size_t> h) override;
  BOOST_ASYNC_DECL void async_write_some_at_impl_(std::uint64_t offset, buffers::const_buffer_subspan buffer,   async::completion_handler<system::error_code, std::size_t> h) override;
  asio::basic_random_access_file<executor> random_access_file_;

};
}
#endif
#endif //BOOST_ASYNC_RANDOM_ACCESS_FILE_HPP
