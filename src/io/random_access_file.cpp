//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#define _FILE_OFFSET_BITS 64

#include <boost/cobalt/io/random_access_file.hpp>


namespace boost::cobalt::io
{
#if defined(BOOST_ASIO_HAS_FILE)

random_access_file::random_access_file(const executor & executor)
    : file(implementation_), implementation_(executor)
{
}

random_access_file::random_access_file(const char * path, file_base::flags open_flags,
                                       const executor & executor)
    : file(implementation_), implementation_(executor, path, open_flags)
{
}

random_access_file::random_access_file(const std::string & path, file_base::flags open_flags,
                                       const executor & executor)
    : file(implementation_), implementation_(executor, path, open_flags)
{
}
random_access_file::random_access_file(const native_handle_type & native_file,
                                       const executor & executor)
    : file(implementation_), implementation_(executor, native_file)
{
}

random_access_file::random_access_file(random_access_file && sf) noexcept = default;

void random_access_file::initiate_read_some_at_(void *this_, std::uint64_t offset,  mutable_buffer_sequence buffer, completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf) {static_cast<random_access_file*>(this_)->implementation_.async_read_some_at(offset, buf, std::move(handler)); });
}
void random_access_file::initiate_write_some_at_(void *this_, std::uint64_t offset, const_buffer_sequence buffer, completion_handler<system::error_code, std::size_t> handler)
{
  visit(buffer, [&](auto buf) {static_cast<random_access_file*>(this_)->implementation_.async_write_some_at(offset, buf, std::move(handler)); });
}

#else

random_access_file::random_access_file(const executor & executor)
    : file(executor)
{

}

random_access_file::random_access_file(const char * path, file::flags open_flags,
                                       const executor & executor)
    : file(executor)
{
  open(path, open_flags).value();
}

random_access_file::random_access_file(const std::string & path, file::flags open_flags,
                                       const executor & executor)
    : file(executor)
{
  open(path, open_flags).value();
}

random_access_file::random_access_file(const native_handle_type & native_file,
                                       const executor & executor)
    : file(executor, native_file)
{
}

random_access_file::random_access_file(random_access_file && sf) noexcept = default;

void initiate_async_read_some_at_helper(boost::asio::posix::basic_stream_descriptor<executor> & str,
                                        std::uint64_t offset, mutable_buffer_sequence seq,
                                        completion_handler<system::error_code, std::size_t> handler)
{

  if (lseek(str.native_handle(), SEEK_SET, offset) < 0)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return handler(system::error_code{errno, ::boost::system::system_category(), &loc}, 0u);
  }

  visit(seq, [&](auto buffer) {str.async_read_some(buffer, std::move(handler)); });
}

void initiate_async_write_some_at_helper(boost::asio::posix::basic_stream_descriptor<executor> & str,
                                        std::uint64_t offset, const_buffer_sequence seq,
                                        completion_handler<system::error_code, std::size_t> handler)
{
  if (lseek(str.native_handle(), SEEK_SET, offset) < 0)
  {
    constexpr static boost::source_location loc{BOOST_CURRENT_LOCATION};
    return handler(system::error_code{errno, ::boost::system::system_category(), &loc}, 0u);
  }

  visit(seq, [&](auto buffer) {str.async_write_some(buffer, std::move(handler)); });
}

void random_access_file::initiate_read_some_at_(void *this_, std::uint64_t offset,  mutable_buffer_sequence buffer, completion_handler<system::error_code, std::size_t> handler)
{
  return initiate_async_read_some_at_helper(static_cast<random_access_file*>(this_)->file_, offset, buffer, std::move(handler));
}
void random_access_file::initiate_write_some_at_(void *this_, std::uint64_t offset, const_buffer_sequence buffer, completion_handler<system::error_code, std::size_t> handler)
{
  return initiate_async_write_some_at_helper(static_cast<random_access_file*>(this_)->file_, offset, buffer, std::move(handler));
}

#endif

}

