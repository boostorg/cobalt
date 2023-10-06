//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/async/io/file.hpp>
#if defined(BOOST_ASIO_HAS_FILE)

namespace boost::async::io
{


system::result<void> file::open(core::string_view file, flags open_flags)
{
  system::error_code ec;
  file_.open(std::string(file), open_flags, ec);
  return ec ? ec : system::result<void>{};
}


system::result<void> file::close()
{
  system::error_code ec;
  file_.close(ec);
  return ec ? ec : system::result<void>{};
}

system::result<void> file::cancel()
{
  system::error_code ec;
  file_.cancel(ec);
  return ec ? ec : system::result<void>{};
}


bool file::is_open() const
{
  return file_.is_open();
}
system::result<void> file::resize(std::uint64_t size)
{
  system::error_code ec;
  file_.resize(size, ec);
  return ec ? ec : system::result<void>{};
}


system::result<std::uint64_t> file::size() const
{
  system::error_code ec;
  auto res = file_.size(ec);
  return ec ? ec : system::result<std::uint64_t>(res);
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

system::result<void> file::assign(native_handle_type native_handle)
{
  system::error_code ec;
  file_.assign(native_handle, ec);
  return ec ? ec : system::result<void>{};
}

system::result<file::native_handle_type> file::release()
{
  system::error_code ec;
  auto h = file_.release(ec);
  return ec ? ec : system::result<native_handle_type>(h);
}

}
#endif