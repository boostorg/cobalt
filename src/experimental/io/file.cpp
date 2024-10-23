//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/cobalt/experimental/io/file.hpp>

#if defined(BOOST_ASIO_HAS_FILE)


namespace boost::cobalt::experimental::io
{

system::result<void> file::assign(const native_handle_type & native_file)
{
  system::error_code ec;
  file_.assign(native_file, ec);
  return ec ? ec : system::result<void>{};
}
system::result<void> file::cancel()
{
  system::error_code ec;
  file_.cancel(ec);
  return ec ? ec : system::result<void>{};
}

executor file::get_executor() { return file_.get_executor();}
bool file::is_open() const { return file_.is_open();}

auto file::native_handle() -> native_handle_type {return file_.native_handle();}

system::result<void> file::open(const char * path, flags open_flags)
{
  system::error_code ec;
  file_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> file::open(const std::string & path, flags open_flags)
{
  system::error_code ec;
  file_.open(path, open_flags, ec);
  return ec ? ec : system::result<void>{};
}

auto file::release() -> system::result<native_handle_type>
{
  system::error_code ec;
  auto r = file_.release(ec);
  return ec ? ec : system::result<native_handle_type>{r};
}

system::result<std::uint64_t> file::size() const
{
  system::error_code ec;
  auto n = file_.size(ec);
  return ec ? ec : system::result<std::uint64_t>{n};
}


system::result<void> file::resize(std::uint64_t sz)
{
  system::error_code ec;
  file_.resize(sz, ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> file::sync_all()
{
  system::error_code ec;
  file_.sync_all(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> file::sync_data()
{
  system::error_code ec;
  file_.sync_data(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> file::close()
{
  system::error_code ec;
  file_.close(ec);
  return ec ? ec : system::result<void>{};
}

}

#endif
