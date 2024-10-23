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
    : implementation_(executor)
{
}

random_access_file::random_access_file(const char * path, file_base::flags open_flags,
                                       const cobalt::executor & executor)
    : implementation_(executor, path, open_flags)
{
}

random_access_file::random_access_file(const std::string & path, file_base::flags open_flags,
                                       const cobalt::executor & executor)
    : implementation_(executor, path, open_flags)
{
}
random_access_file::random_access_file(const native_handle_type & native_file,
                                       const cobalt::executor & executor)
    : implementation_(executor, native_file)
{
}

random_access_file::random_access_file(random_access_file && sf) noexcept = default;

system::result<void> random_access_file::assign(const native_handle_type & native_file)
{
  system::error_code ec;
  implementation_.assign(native_file, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> random_access_file::cancel()
{
  system::error_code ec;
  implementation_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

executor random_access_file::get_executor() { return implementation_.get_executor();}
bool random_access_file::is_open() const { return implementation_.is_open();}

auto random_access_file::native_handle() -> native_handle_type {return implementation_.native_handle();}

system::result<void> random_access_file::open(const char * path, flags open_flags)
{
  system::error_code ec;
  implementation_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> random_access_file::open(const std::string & path, flags open_flags)
{
  system::error_code ec;
  implementation_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

auto random_access_file::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto r = implementation_.release(ec);
  return ec ? ec : system::result<native_handle_type>{r};
}

system::result<void> random_access_file::resize(std::uint64_t n)
{
  system::error_code ec;
  implementation_.resize(n, ec);
  return ec ? ec : system::result<void>{};
}


system::result<std::uint64_t> random_access_file::size() const
{
  system::error_code ec;
  auto n = implementation_.size(ec);
  return ec ? ec : system::result<std::uint64_t>{n};
}

system::result<void> random_access_file::sync_all()
{
  system::error_code ec;
  implementation_.sync_all(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> random_access_file::sync_data()
{
  system::error_code ec;
  implementation_.sync_data(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> random_access_file::close()
{
  system::error_code ec;
  implementation_.close(ec);
  return ec ? ec : system::result<void>{};
}

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