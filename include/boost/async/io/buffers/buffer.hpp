//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_BUFFER_HPP
#define BOOST_BUFFERS_BUFFER_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <cstdlib>
#include <type_traits>

namespace boost::async::io::buffers {

/** Return a buffer.
*/
inline
mutable_buffer
buffer(
    mutable_buffer const& b) noexcept
{
    return b;
}

/** Return a buffer.
*/
inline
mutable_buffer
buffer(
    void* data,
    std::size_t size) noexcept
{
    return mutable_buffer(data, size);
}

/** Return a buffer.
*/
inline
const_buffer
buffer(
    const_buffer const& b) noexcept
{
    return b;
}

/** Return a buffer.
*/
inline
const_buffer
buffer(
    void const* data,
    std::size_t size) noexcept
{
    return const_buffer(data, size);
}

/** Return a buffer.
*/
template<class T, std::size_t N>
  requires std::is_trivially_copyable<T>::value
mutable_buffer
buffer(
    T (&data)[N]) noexcept
{
    return mutable_buffer(
        data, N * sizeof(T));
}

/** Return a buffer.
*/
template<class T, std::size_t N>
  requires std::is_trivially_copyable<T>::value
const_buffer
buffer(
    T const (&data)[N]) noexcept
{
    return const_buffer(
        data, N * sizeof(T));
}

} // boost::buffers

#endif
