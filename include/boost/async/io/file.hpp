//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_FILE_HPP
#define BOOST_ASYNC_IO_FILE_HPP

#include <boost/asio/basic_file.hpp>

#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/async/config.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>

namespace boost::async::io
{


struct file
{
  using seek_basis = boost::asio::file_base::seek_basis;
  using flags      = boost::asio::file_base::flags;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> open(core::string_view file,
                                          flags open_flags = flags::read_write);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> close();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> cancel();
  [[nodiscard]] BOOST_ASYNC_DECL bool is_open() const;
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> resize(std::uint64_t size);
  [[nodiscard]] BOOST_ASYNC_DECL system::result<std::uint64_t> size() const;

  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> sync_all();
  [[nodiscard]] BOOST_ASYNC_DECL system::result<void> sync_data();


  using native_handle_type = asio::basic_file<executor_type>::native_handle_type;
  BOOST_ASYNC_DECL native_handle_type native_handle();

  BOOST_ASYNC_DECL system::result<void> assign(native_handle_type native_handle);
  BOOST_ASYNC_DECL system::result<native_handle_type> release();

 private:
  asio::basic_file<executor_type> & file_;
 public:
  file(asio::basic_file<executor_type> & file) : file_(file) {}
};

}

#endif
#endif //BOOST_ASYNC_IO_FILE_HPP
