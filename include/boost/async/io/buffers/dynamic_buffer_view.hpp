//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_BUFFER_DYNAMIC_BUFFER_VIEW_HPP
#define BOOST_ASYNC_IO_BUFFER_DYNAMIC_BUFFER_VIEW_HPP

#include <boost/async/io/buffers/algorithm.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/const_buffer_pair.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer_pair.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <boost/async/io/buffers/concepts.hpp>

#include <boost/asio/socket_base.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/container_fwd.hpp>
#include <boost/circular_buffer_fwd.hpp>

#include <deque>

namespace boost::async::io::buffers
{

struct dynamic_buffer_view
{
  using mutable_buffers_type = container::static_vector<mutable_buffer, boost::asio::detail::max_iov_len>;
  using   const_buffers_type = container::static_vector<  const_buffer, boost::asio::detail::max_iov_len>;

  std::size_t          size    () const        { return vtable_.size    (this_, position_); }
  std::size_t          max_size() const        { return vtable_.max_size(this_); }
  std::size_t          capacity() const        { return vtable_.capacity(this_); }
  const_buffers_type   data    () const        { return vtable_.data    (this_, position_); }
  mutable_buffers_type prepare (std::size_t n) { return vtable_.prepare (this_, n, position_); }
  void                 commit  (std::size_t n) { return vtable_.commit  (this_, n, position_); }
  void                 consume (std::size_t n) { return vtable_.consume (this_, n, position_); }

  dynamic_buffer_view(const dynamic_buffer_view&) noexcept = default;
  dynamic_buffer_view(dynamic_buffer_view&&)      noexcept =default;

  template<dynamic_buffer Buffer>
  dynamic_buffer_view(Buffer &other);

  template<typename Container >
    requires (
        requires (const Container & ct)
        {
          {ct.data()} -> std::convertible_to<const void*>;
          {ct.size()} -> std::convertible_to<std::size_t>;
          {ct.max_size()} -> std::convertible_to<std::size_t>;
          {ct.capacity()} -> std::convertible_to<std::size_t>;
        }
        &&        requires (Container & ct)
        {
          {ct.data()} -> std::convertible_to<void*>;
          {ct.resize(std::size_t())};
          {ct.erase(ct.begin(), ct.end())};
        }
        && std::is_trivial_v<typename Container::value_type>
        && sizeof(typename Container::value_type) == 1u)
  dynamic_buffer_view(Container  &other);

  template<typename T, typename Allocator>
    requires (std::is_trivial_v<T> && sizeof(T) == 1u)
  dynamic_buffer_view(boost::circular_buffer<T, Allocator> &other);

  template<typename T, typename Allocator, typename Options>
  requires (std::is_trivial_v<T> && sizeof(T) == 1u)
  dynamic_buffer_view(boost::container::deque<T, Allocator, Options> &other);

 private:
  struct vtable_t
  {
    std::size_t          (*size)    (const void * this_, std::size_t pos) = 0;
    std::size_t          (*max_size)(const void * this_) = 0;
    std::size_t          (*capacity)(const void * this_) = 0;
    const_buffers_type   (*data)    (const void * this_, std::size_t pos) = 0;
    mutable_buffers_type (*prepare) (void * this_, std::size_t n, std::size_t pos) = 0;
    void                 (*commit)  (void * this_, std::size_t n, std::size_t & pos) = 0;
    void                 (*consume) (void * this_, std::size_t n, std::size_t & pos) = 0;
  };
  struct vtables;

  void* this_;
  const vtable_t & vtable_;
  std::size_t position_{0u};
};






}

#include <boost/async/io/buffers/impl/dynamic_buffer_view.hpp>

#endif //BOOST_ASYNC_IO_BUFFER_DYNAMIC_BUFFER_VIEW_HPP
