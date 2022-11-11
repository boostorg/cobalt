//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_FILE_IPP
#define BOOST_ASYNC_IO_IMPL_FILE_IPP

#if defined(BOOST_ASIO_HAS_FILE)

#include <boost/async/io/file.hpp>

namespace boost::async::io
{
file::file(file&& other) = default;
file& file::operator=(file&& other) = default;

auto file::get_executor() noexcept -> executor_type {return impl_->get_executor();};

                   void file::open(const char* path, file_base::flags open_flags)                                {        impl_->open(path, open_flags); }
BOOST_ASIO_SYNC_OP_VOID file::open(const char* path, file_base::flags open_flags, boost::system::error_code& ec) { return impl_->open(path, open_flags, ec); }
                   void file::open(const std::string& path, file_base::flags open_flags)                                { return impl_->open(path, open_flags); }
BOOST_ASIO_SYNC_OP_VOID file::open(const std::string& path, file_base::flags open_flags, boost::system::error_code& ec) { return impl_->open(path, open_flags, ec); }
void file::assign(const native_handle_type& native_file)                                { impl_->assign(native_file); }
void file::assign(const native_handle_type& native_file, boost::system::error_code& ec) { impl_->assign(native_file, ec); }
bool file::is_open() const { return impl_->is_open(); }
void file::close()                              { impl_->close(); }
void file::close(boost::system::error_code& ec) { impl_->close(ec); }
auto file::release()                              -> native_handle_type { return impl_->release(); }
auto file::release(boost::system::error_code& ec) -> native_handle_type { return impl_->release(ec); }
auto file::native_handle() -> native_handle_type { return impl_->native_handle(); }    
void file::cancel()                              { impl_->cancel(); }    
void file::cancel(boost::system::error_code& ec) { impl_->cancel(ec); }    
uint64_t file::size()                              const { return impl_->size(); }   
uint64_t file::size(boost::system::error_code& ec) const { return impl_->size(ec); }    
                   void file::resize(uint64_t n)                                {        impl_->resize(n);   }
BOOST_ASIO_SYNC_OP_VOID file::resize(uint64_t n, boost::system::error_code& ec) { return impl_->resize(n, ec); }
                   void file::sync_all()                              {        impl_->sync_all();     }
BOOST_ASIO_SYNC_OP_VOID file::sync_all(boost::system::error_code& ec) { return impl_->sync_all(ec);   }
                   void file::sync_data()                              {        impl_->sync_data();   }
BOOST_ASIO_SYNC_OP_VOID file::sync_data(boost::system::error_code& ec) { return impl_->sync_data(ec); }

file::file() = default;
file::~file() = default;

stream_file::stream_file(asio::io_context& context)                                                             : impl_(context)              { file::impl_ = &impl_; }
stream_file::stream_file(asio::io_context& context, const char* path, asio::file_base::flags open_flags)        : impl_(context, open_flags)  { file::impl_ = &impl_; }
stream_file::stream_file(asio::io_context& context, const std::string& path, asio::file_base::flags open_flags) : impl_(context, open_flags)  { file::impl_ = &impl_; }
stream_file::stream_file(asio::io_context& context, const native_handle_type& native_file)                      : impl_(context, native_file) { file::impl_ = &impl_; }
stream_file::stream_file(const executor_type& ex)                                                               : impl_(ex)              { file::impl_ = &impl_; }
stream_file::stream_file(const executor_type& ex, const char* path, asio::file_base::flags open_flags)          : impl_(ex, open_flags)  { file::impl_ = &impl_; }
stream_file::stream_file(const executor_type& ex, const std::string& path, asio::file_base::flags open_flags)   : impl_(ex, open_flags)  { file::impl_ = &impl_; }
stream_file::stream_file(const executor_type& ex, const native_handle_type& native_file)                        : impl_(ex, native_file) { file::impl_ = &impl_; }

stream_file::stream_file(stream_file&& other) noexcept = default;
stream_file& stream_file::operator=(stream_file&& other) = default;
stream_file::~stream_file() = default;


uint64_t stream_file::seek(int64_t offset, asio::file_base::seek_basis whence)                                { return impl_.seek(offset, whence); }
uint64_t stream_file::seek(int64_t offset, asio::file_base::seek_basis whence, boost::system::error_code& ec) { return impl_.seek(offset, whence, ec); }


void stream_file::async_write_some(asio::const_buffer buffer,     concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h)); }
void stream_file::async_write_some(prepared_buffers   buffer,     concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h)); }
void stream_file::async_write_some(any_const_buffer_range buffer, concepts::write_handler h) { return impl_.async_write_some(buffer, std::move(h));}


void stream_file::async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler h) { return impl_.async_read_some(buffer, std::move(h)); }
void stream_file::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h) { return impl_.async_read_some(buffer, std::move(h)); }
void stream_file::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h) { return impl_.async_read_some(buffer, std::move(h)); }


random_access_file::random_access_file(asio::io_context& context)                                                             : impl_(context)              { file::impl_ = &impl_; }
random_access_file::random_access_file(asio::io_context& context, const char* path, asio::file_base::flags open_flags)        : impl_(context, open_flags)  { file::impl_ = &impl_; }
random_access_file::random_access_file(asio::io_context& context, const std::string& path, asio::file_base::flags open_flags) : impl_(context, open_flags)  { file::impl_ = &impl_; }
random_access_file::random_access_file(asio::io_context& context, const native_handle_type& native_file)                      : impl_(context, native_file) { file::impl_ = &impl_; }
random_access_file::random_access_file(const executor_type& ex)                                                               : impl_(ex)              { file::impl_ = &impl_; }
random_access_file::random_access_file(const executor_type& ex, const char* path, asio::file_base::flags open_flags)          : impl_(ex, open_flags)  { file::impl_ = &impl_; }
random_access_file::random_access_file(const executor_type& ex, const std::string& path, asio::file_base::flags open_flags)   : impl_(ex, open_flags)  { file::impl_ = &impl_; }
random_access_file::random_access_file(const executor_type& ex, const native_handle_type& native_file)                        : impl_(ex, native_file) { file::impl_ = &impl_; }

random_access_file::random_access_file(random_access_file&& other) noexcept = default;
random_access_file& random_access_file::operator=(random_access_file&& other) = default;

random_access_file::~random_access_file() = default;

void random_access_file::async_write_some_at(std::uint64_t offset, asio::const_buffer buffer, concepts::write_handler h) { impl_.async_write_some_at(offset, buffer, h); }
void random_access_file::async_read_some_at(std::uint64_t offset, asio::mutable_buffer buffer, concepts::read_handler h) { impl_.async_read_some_at(offset, buffer, h); }






}
#endif


#endif //BOOST_ASYNC_IO_IMPL_FILE_IPP
