//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_MATCH_CONDITION_HPP
#define BOOST_ASYNC_IO_MATCH_CONDITION_HPP

#include <boost/asio/buffers_iterator.hpp>

namespace boost::async::io
{

template<typename Buffer>
struct match_condition_base
{
  using iterator = asio::buffers_iterator<typename Buffer::const_buffers_type>;
  virtual std::pair<iterator, bool> operator()(iterator begin, iterator end) = 0;
};

template<>
struct match_condition_base<std::string>
{
  using buffer_type = asio::dynamic_string_buffer<char, std::char_traits<char>, std::allocator<char>>;
  using iterator = asio::buffers_iterator<typename buffer_type::const_buffers_type>;
  virtual std::pair<iterator, bool> operator()(iterator begin, iterator end) = 0;
};

template<typename T>
struct match_condition_base<std::vector<T>>
{
  using buffer_type = asio::dynamic_vector_buffer<T, std::allocator<T>>;
  using iterator = asio::buffers_iterator<typename buffer_type::const_buffers_type>;
  virtual std::pair<iterator, bool> operator()(iterator begin, iterator end) = 0;
};



template<typename Buffer>
struct match_condition
{
  using iterator = asio::buffers_iterator<typename Buffer::const_buffers_type>;
  using result_type = std::pair<iterator, bool>;
  std::pair<iterator, bool> operator()(iterator begin, iterator end)
  {
    return (*cc)(begin, end);
  }
  match_condition_base<Buffer> * cc;
};

}

#endif //BOOST_ASYNC_IO_MATCH_CONDITION_HPP
