//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/initiate_templates.hpp>
#include <boost/cobalt/experimental/io/random_access_file.hpp>

#if defined(BOOST_ASIO_HAS_FILE)

namespace boost::cobalt::experimental::io
{


random_access_file::random_access_file(const cobalt::executor & executor)
    : file(implementation_), implementation_(executor)
{
}

random_access_file::random_access_file(const char * path, file_base::flags open_flags,
                                       const cobalt::executor & executor)
    : file(implementation_), implementation_(executor, path, open_flags)
{
}

random_access_file::random_access_file(const std::string & path, file_base::flags open_flags,
                                       const cobalt::executor & executor)
    : file(implementation_), implementation_(executor, path, open_flags)
{
}
random_access_file::random_access_file(const native_handle_type & native_file,
                                       const cobalt::executor & executor)
    : file(implementation_), implementation_(executor, native_file)
{
}

random_access_file::random_access_file(random_access_file && sf) noexcept = default;

void random_access_file::initiate_read_some_at_(void *this_, std::uint64_t offset,  mutable_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  return initiate_async_read_some_at(static_cast<random_access_file*>(this_)->implementation_, offset, buffer, std::move(handler));
}
void random_access_file::initiate_write_some_at_(void *this_, std::uint64_t offset, const_buffer_sequence buffer, boost::cobalt::completion_handler<boost::system::error_code, std::size_t> handler)
{
  return initiate_async_write_some_at(static_cast<random_access_file*>(this_)->implementation_, offset, buffer, std::move(handler));
}


}

#endif