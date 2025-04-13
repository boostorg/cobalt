//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define _FILE_OFFSET_BITS 64

#include <boost/cobalt/io/stream_file.hpp>


namespace boost::cobalt::io
{

#if defined(BOOST_ASIO_HAS_FILE)

stream_file::stream_file(const cobalt::executor & executor)
      : file(implementation_), implementation_(executor)
{
}

stream_file::stream_file(const char * path, file_base::flags open_flags,
                         const cobalt::executor & executor)
      : file(implementation_), implementation_(executor, path, open_flags)
{
}

stream_file::stream_file(const std::string & path, file_base::flags open_flags,
                         const cobalt::executor & executor)
      : file(implementation_), implementation_(executor, path, open_flags)
{
}
stream_file::stream_file(const native_handle_type & native_file,
                         const cobalt::executor & executor)
      : file(implementation_), implementation_(executor, native_file)
{
}

stream_file::stream_file(stream_file && sf) noexcept = default;


system::result<std::uint64_t> stream_file::seek(std::int64_t offset, seek_basis whence)
{
  system::error_code ec;
  auto n = implementation_.seek(offset, whence);
  return ec ? ec : system::result<std::uint64_t>{n};
}

void stream_file::initiate_read_some_(void * this_, mutable_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buf)
        {
          static_cast<stream_file*>(this_)->implementation_.async_read_some(buf, std::move(handler));
        });

}
void stream_file::initiate_write_some_(void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer,
        [&](auto buf)
        {
          static_cast<stream_file*>(this_)->implementation_.async_write_some(buf, std::move(handler));
        });
}

#else


stream_file::stream_file(const cobalt::executor & executor) : file(executor)
{
}

stream_file::stream_file(const char * path, file::flags open_flags,
                         const cobalt::executor & executor)
    : file(executor)
{
  open(path, open_flags).value();
}

stream_file::stream_file(const std::string & path, file::flags open_flags,
                         const cobalt::executor & executor) : file(executor)
{
  open(path, open_flags).value();
}
stream_file::stream_file(const native_handle_type & native_file,
                         const cobalt::executor & executor)
    : file(executor, native_file)
{
}

stream_file::stream_file(stream_file && sf) noexcept = default;


#if !defined(COBALT_RETURN_ERROR)
#define COBALT_RETURN_ERROR()                                            \
  do {                                                                   \
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION}; \
    return system::error_code{errno, ::boost::system::system_category(), &loc};  \
  }                                                                      \
  while(true)
#endif

system::result<std::uint64_t> stream_file::seek(std::int64_t offset, seek_basis whence)
{
  if (::lseek(native_handle(), offset, whence) < 0)
    COBALT_RETURN_ERROR();

  return {};
}

void stream_file::initiate_read_some_(void * this_, mutable_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf) {static_cast<stream_file*>(this_)->file_.async_read_some(buf, std::move(handler)); });
}
void stream_file::initiate_write_some_(void * this_, const_buffer_sequence buffer, boost::cobalt::completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf) {static_cast<stream_file*>(this_)->file_.async_write_some(buf, std::move(handler)); });
}

#endif
}

