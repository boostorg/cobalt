//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_IMPL_PIPE_IPP
#define BOOST_ASYNC_IO_IMPL_PIPE_IPP

#include <boost/async/io/pipe.hpp>


namespace boost::async::io
{

readable_pipe::readable_pipe(const executor_type &ex)
  : impl_(ex) {}
readable_pipe::readable_pipe(asio::io_context &context)
  : impl_(context) {}
readable_pipe::readable_pipe(const executor_type &ex, const native_handle_type &native_pipe)
  : impl_(ex, native_pipe) {}
readable_pipe::readable_pipe(asio::io_context &context, const native_handle_type &native_pipe)
  : impl_(context, native_pipe) {}

readable_pipe::readable_pipe(readable_pipe &&other) = default;
readable_pipe &readable_pipe::operator=(readable_pipe &&other) = default;
readable_pipe::~readable_pipe() = default;


auto readable_pipe::get_executor() noexcept -> executor_type          {return impl_.get_executor();}
auto readable_pipe::lowest_layer() -> lowest_layer_type &             {return impl_.lowest_layer();}
auto readable_pipe::lowest_layer() const -> const lowest_layer_type & {return impl_.lowest_layer();}

void readable_pipe::assign(const native_handle_type &native_pipe) { impl_.assign(native_pipe);}
void readable_pipe::assign(const native_handle_type &native_pipe, boost::system::error_code &ec) { impl_.assign(native_pipe, ec); }
bool readable_pipe::is_open() const { return impl_.is_open();}
void readable_pipe::close()  { impl_.close();}
void readable_pipe::close(boost::system::error_code &ec) { impl_.close(ec);}
auto readable_pipe::release() -> native_handle_type { return impl_.release(); }
auto readable_pipe::release(boost::system::error_code &ec) -> native_handle_type { return impl_.release(ec); }
auto readable_pipe::native_handle() -> native_handle_type { return impl_.native_handle(); }
void readable_pipe::cancel() {impl_.cancel();}
void readable_pipe::cancel(boost::system::error_code &ec) {impl_.cancel(ec);}

void readable_pipe::async_read_some(asio::mutable_buffer buffer,                     concepts::write_handler  h)
{
  impl_.async_read_some(buffer, std::move(h));
}
void readable_pipe::async_read_some(static_buffer_base::mutable_buffers_type buffer, concepts::write_handler h)
{
  impl_.async_read_some(buffer, std::move(h));
}
void readable_pipe::async_read_some(multi_buffer::mutable_buffers_type buffer,       concepts::write_handler h)
{
  impl_.async_read_some(buffer, std::move(h));
}


writable_pipe::writable_pipe(const executor_type &ex)
    : impl_(ex) {}
writable_pipe::writable_pipe(asio::io_context &context)
    : impl_(context) {}
writable_pipe::writable_pipe(const executor_type &ex, const native_handle_type &native_pipe)
    : impl_(ex, native_pipe) {}
writable_pipe::writable_pipe(asio::io_context &context, const native_handle_type &native_pipe)
    : impl_(context, native_pipe) {}

writable_pipe::writable_pipe(writable_pipe &&other) = default;
writable_pipe &writable_pipe::operator=(writable_pipe &&other) = default;
writable_pipe::~writable_pipe() = default;


auto writable_pipe::get_executor() noexcept -> executor_type          {return impl_.get_executor();}
auto writable_pipe::lowest_layer() -> lowest_layer_type &             {return impl_.lowest_layer();}
auto writable_pipe::lowest_layer() const -> const lowest_layer_type & {return impl_.lowest_layer();}

void writable_pipe::assign(const native_handle_type &native_pipe) { impl_.assign(native_pipe);}
void writable_pipe::assign(const native_handle_type &native_pipe, boost::system::error_code &ec) { impl_.assign(native_pipe, ec); }
bool writable_pipe::is_open() const { return impl_.is_open();}
void writable_pipe::close()  { impl_.close();}
void writable_pipe::close(boost::system::error_code &ec) { impl_.close(ec);}
auto writable_pipe::release() -> native_handle_type { return impl_.release(); }
auto writable_pipe::release(boost::system::error_code &ec) -> native_handle_type { return impl_.release(ec); }
auto writable_pipe::native_handle() -> native_handle_type { return impl_.native_handle(); }
void writable_pipe::cancel() {impl_.cancel();}
void writable_pipe::cancel(boost::system::error_code &ec) {impl_.cancel(ec);}


void writable_pipe::async_write_some(
    asio::const_buffer buffer,
    boost::async::detail::completion_handler<system::error_code, std::size_t> h)
{
  impl_.async_write_some(buffer, std::move(h));
}

void writable_pipe::async_write_some(prepared_buffers buffer, concepts::write_handler h)
{
  impl_.async_write_some(buffer, std::move(h));
}

void connect_pipe(readable_pipe& read_end, writable_pipe& write_end)
{
  asio::connect_pipe(read_end.implementation(), write_end.implementation());
}

void connect_pipe(readable_pipe& read_end, writable_pipe& write_end, boost::system::error_code& ec)
{
  asio::connect_pipe(read_end.implementation(), write_end.implementation(), ec);
}
auto connect_pipe(asio::io_context::executor_type executor) -> std::pair<readable_pipe, writable_pipe>
{
  readable_pipe read_end{executor};
  writable_pipe write_end{executor};

  connect_pipe(read_end, write_end);
  return {std::move(read_end), std::move(write_end)};
}

auto connect_pipe(asio::io_context::executor_type executor, boost::system::error_code& ec) -> std::pair<readable_pipe, writable_pipe>
{
  readable_pipe read_end{executor};
  writable_pipe write_end{executor};

  connect_pipe(read_end, write_end, ec);
  return {std::move(read_end), std::move(write_end)};
}


}

#endif //BOOST_ASYNC_IO_IMPL_PIPE_IPP
