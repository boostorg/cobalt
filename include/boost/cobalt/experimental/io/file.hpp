//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_IO_FILE_HPP
#define BOOST_COBALT_EXPERIMENTAL_IO_FILE_HPP

#include <boost/asio/basic_file.hpp>
#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/cobalt/config.hpp>
#include <boost/system/result.hpp>

namespace boost::cobalt::experimental::io
{

struct file : boost::asio::file_base
{

  using native_handle_type = asio::basic_file<executor>::native_handle_type;

  BOOST_COBALT_DECL system::result<void> assign(const native_handle_type & native_file);
  BOOST_COBALT_DECL system::result<void> cancel();

  BOOST_COBALT_DECL executor get_executor();
  BOOST_COBALT_DECL bool is_open() const;

  BOOST_COBALT_DECL system::result<void> close();
  BOOST_COBALT_DECL native_handle_type native_handle();

  BOOST_COBALT_DECL system::result<void> open(const char * path,        flags open_flags);
  BOOST_COBALT_DECL system::result<void> open(const std::string & path, flags open_flags);

  BOOST_COBALT_DECL system::result<native_handle_type> release();
  BOOST_COBALT_DECL system::result<void> resize(std::uint64_t n);

  BOOST_COBALT_DECL system::result<std::uint64_t> size() const;
  BOOST_COBALT_DECL system::result<void> sync_all();
  BOOST_COBALT_DECL system::result<void> sync_data();


  file(asio::basic_file<executor> & file) : file_(file) {}
 private:
  asio::basic_file<executor> & file_;
};

}

#endif

#endif //BOOST_COBALT_EXPERIMENTAL_IO_FILE_HPP
