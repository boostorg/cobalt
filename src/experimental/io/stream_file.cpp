//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/initiate_templates.hpp>
#include <boost/cobalt/experimental/io/stream_file.hpp>
#include <boost/cobalt/experimental/io/stream.hpp>

#if defined(BOOST_ASIO_HAS_FILE)

namespace boost::cobalt::experimental::io
{

static_assert(stream<stream_file>);


stream_file::stream_file()
      : implementation_(this_thread::get_executor())
{
}

stream_file::stream_file(const char * path, file_base::flags open_flags)
      : implementation_(this_thread::get_executor(), path, open_flags)
{
}

stream_file::stream_file(const std::string & path, file_base::flags open_flags)
      : implementation_(this_thread::get_executor(), path, open_flags)
{
}
stream_file::stream_file(const native_handle_type & native_file)
      : implementation_(this_thread::get_executor(), native_file)
{
}

stream_file::stream_file(stream_file && sf) noexcept = default;

system::result<void> stream_file::assign(const native_handle_type & native_file)
{
  system::error_code ec;
  implementation_.assign(native_file, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> stream_file::cancel()
{
  system::error_code ec;
  implementation_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

executor stream_file::get_executor() { return implementation_.get_executor();}
bool stream_file::is_open() const { return implementation_.is_open();}

auto stream_file::native_handle() -> native_handle_type {return implementation_.native_handle();}

system::result<void> stream_file::open(const char * path, flags open_flags)
{
  system::error_code ec;
  implementation_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> stream_file::open(const std::string & path, flags open_flags)
{
  system::error_code ec;
  implementation_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

auto stream_file::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto r = implementation_.release(ec);
  return ec ? ec : system::result<native_handle_type>{r};
}

system::result<void> stream_file::resize(std::uint64_t n)
{
  system::error_code ec;
  implementation_.resize(n, ec);
  return ec ? ec : system::result<void>{};
}

system::result<std::uint64_t> stream_file::seek(std::int64_t offset, seek_basis whence)
{
  system::error_code ec;
  auto n = implementation_.seek(offset, whence);
  return ec ? ec : system::result<std::uint64_t>{n};
}

system::result<std::uint64_t> stream_file::size() const
{
  system::error_code ec;
  auto n = implementation_.size(ec);
  return ec ? ec : system::result<std::uint64_t>{n};
}

system::result<void> stream_file::sync_all()
{
  system::error_code ec;
  implementation_.sync_all(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> stream_file::sync_data()
{
  system::error_code ec;
  implementation_.sync_data(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> stream_file::close()
{
  system::error_code ec;
  implementation_.close(ec);
  return ec ? ec : system::result<void>{};
}
void stream_file::initiate_read_some_(void * this_, mutable_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  initiate_async_read_some(static_cast<stream_file*>(this_)->implementation_, buffer, std::move(handler));
}
void stream_file::initiate_write_some_(void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  initiate_async_write_some(static_cast<stream_file*>(this_)->implementation_, buffer, std::move(handler));
}


}

#endif