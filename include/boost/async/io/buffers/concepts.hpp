//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_CONCEPTS_HPP
#define BOOST_ASYNC_IO_BUFFERS_CONCEPTS_HPP

#include <boost/async/config.hpp>
#include <type_traits>

namespace boost::async::io::buffers {

#ifndef BOOST_ASYNC_IO_BUFFERS_DOCS
class const_buffer;
class mutable_buffer;
#endif

// https://www.boost.org/doc/libs/1_65_0/doc/html/boost_asio/reference/ConstBufferSequence.html

/** Determine if T is a ConstBuffers.
*/
#if BOOST_ASYNC_IO_BUFFERS_DOCS
template<class T>
struct is_const_buffer_sequence
    : std::integral_constant<bool, ...>{};
#else



template<typename T>
concept const_buffer_sequence =
    std::same_as<T, const_buffer> ||
    std::same_as<T, mutable_buffer> ||
   ((std::same_as<typename T::value_type, const_buffer> ||
     std::same_as<typename T::value_type, mutable_buffer>) &&
    std::bidirectional_iterator<typename T::const_iterator> &&
    requires (const T & buf)
    {
      {buf.begin()} -> std::same_as<typename T::const_iterator>;
      {buf.end()}   -> std::same_as<typename T::const_iterator>;

    } &&
    (std::same_as<std::remove_const_t<
        typename std::iterator_traits<typename T::const_iterator>::value_type>,
        const_buffer>
      ||
     std::same_as<std::remove_const_t<
         typename std::iterator_traits<typename T::const_iterator>::value_type>,
         mutable_buffer>)
     )
;


#endif

/** Determine if T is a MutableBuffers.
*/
#if BOOST_ASYNC_IO_BUFFERS_DOCS
template<class T>
struct is_mutable_buffer_sequence
    : std::integral_constant<bool, ...>{};
#else
template<typename T>
concept mutable_buffer_sequence =
    std::same_as<T, mutable_buffer> ||
    (std::same_as<typename T::value_type, mutable_buffer> &&
     std::bidirectional_iterator<typename T::const_iterator> &&
     requires (const T &buf)
     {
       {buf.begin()} -> std::same_as<typename T::const_iterator>;
       {buf.end()}  -> std::same_as<typename T::const_iterator>;
     } &&
    std::same_as<
        std::remove_const_t<typename std::iterator_traits<typename T::const_iterator>::value_type>,
        mutable_buffer>
    )
;


#endif

//------------------------------------------------

/** Determine if T is a DynamicBuffer
*/
#if BOOST_ASYNC_IO_BUFFERS_DOCS
template<class T>
struct is_dynamic_buffer
    : std::integral_constant<bool, ...>{};
#else

template<typename T>
concept dynamic_buffer =
    requires (const T & buf, std::size_t n)
    {
      { buf.size() }     -> std::same_as<std::size_t>;
      { buf.max_size() } -> std::same_as<std::size_t>;
      { buf.capacity() } -> std::same_as<std::size_t>;
    } &&
    requires (T & buf, std::size_t n)
    {

      {buf.commit(n)};
      {buf.consume(n)};
      {buf.data()}    ->   const_buffer_sequence;
      {buf.prepare(n)}-> mutable_buffer_sequence;
    }
    && const_buffer_sequence  <typename T::const_buffers_type>
    && mutable_buffer_sequence<typename T::mutable_buffers_type>
    ;


/** Return the underlying buffer type of a sequence.
*/
template<class T>
using value_type = typename
    std::conditional<
        mutable_buffer_sequence<T>,
        mutable_buffer,
        const_buffer
            >::type;

#endif

template<typename T>
concept buffer_byte = (sizeof(T) == 1u) && std::is_trivial_v<T>;


} // boost::buffers

#endif
