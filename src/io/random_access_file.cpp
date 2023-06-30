//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/random_access_file.hpp>

#if defined(BOOST_ASIO_HAS_FILE)
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<random_access_file> random_access_file::duplicate()
{
  auto res = detail::duplicate_handle(random_access_file_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, random_access_file(*res)};
}



random_access_file::random_access_file()
    : file(random_access_file_), random_access_file_(this_thread::get_executor())
{
}

random_access_file::random_access_file(native_handle_type h)
    : file(random_access_file_), random_access_file_(this_thread::get_executor())
{
}

random_access_file::random_access_file(random_access_file && lhs)
    : file(random_access_file_), random_access_file_(std::move(lhs.random_access_file_))
{
}

random_access_file::random_access_file(core::string_view file_, flags open_flags)
    : file(random_access_file_), random_access_file_(this_thread::get_executor(), std::string(file_), open_flags)
{
}


void random_access_file::async_read_some_at_impl_(
    std::uint64_t offset,
    buffers::mutable_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  random_access_file_.async_read_some_at(offset, buffer, std::move(h));
}

void random_access_file::async_write_some_at_impl_(
    std::uint64_t offset,
    buffers::const_buffer_span buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  random_access_file_.async_write_some_at(offset, buffer, std::move(h));
}

system::result<void> random_access_file::close() { return file::close(); }
system::result<void> random_access_file::cancel() { return file::cancel(); }
bool random_access_file::is_open() const {return file::is_open();}

}

#endif