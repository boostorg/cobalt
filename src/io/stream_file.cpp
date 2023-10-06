//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/stream_file.hpp>

#if defined(BOOST_ASIO_HAS_FILE)
#include <boost/async/io/detail/duplicate.hpp>

namespace boost::async::io
{

system::result<stream_file> stream_file::duplicate(const async::executor & exec)
{
  auto res = detail::io::duplicate_handle(stream_file_.native_handle());
  if (!res)
    return res.error();

  return {system::in_place_value, stream_file(*res, exec)};
}



stream_file::stream_file(const async::executor & exec)
    : file(stream_file_), stream_file_(exec)
{
}

stream_file::stream_file(native_handle_type h, const async::executor & exec)
    : file(stream_file_), stream_file_(exec)
{
}

stream_file::stream_file(stream_file && lhs)
    : file(stream_file_), stream_file_(std::move(lhs.stream_file_))
{
}

stream_file::stream_file(core::string_view file_, flags open_flags, const async::executor & exec)
    : file(stream_file_), stream_file_(exec, std::string(file_), open_flags)
{
}


void stream_file::async_read_some_impl_(
    buffers::mutable_buffer_subspan buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  stream_file_.async_read_some(buffer, std::move(h));
}

void stream_file::async_write_some_impl_(
    buffers::const_buffer_subspan buffer,
    async::completion_handler<system::error_code, std::size_t> h)
{
  stream_file_.async_write_some(buffer, std::move(h));
}

system::result<void> stream_file::close() { return file::close(); }
system::result<void> stream_file::cancel() { return file::cancel(); }
bool stream_file::is_open() const {return file::is_open();}

}

#endif