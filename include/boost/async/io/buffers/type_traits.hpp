//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_TYPE_TRAITS_HPP
#define BOOST_BUFFERS_TYPE_TRAITS_HPP

#include <boost/async/config.hpp>
#include <boost/async/detail/type_traits.hpp>
#include <type_traits>

namespace boost::async::io::buffers {

#ifndef BOOST_BUFFERS_DOCS
class const_buffer;
class mutable_buffer;
#endif

// https://www.boost.org/doc/libs/1_65_0/doc/html/boost_asio/reference/ConstBufferSequence.html

/** Determine if T is a ConstBuffers.
*/
#if BOOST_BUFFERS_DOCS
template<class T>
struct is_const_buffer_sequence
    : std::integral_constant<bool, ...>{};
#else

template<class T, class = void>
struct is_const_buffer_sequence
    : std::false_type
{
};

template<class T>
struct is_const_buffer_sequence<T const>
    : is_const_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_const_buffer_sequence<T const&>
    : is_const_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_const_buffer_sequence<T&>
    : is_const_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<>
struct is_const_buffer_sequence<
    const_buffer>
    : std::true_type
{
};

template<>
struct is_const_buffer_sequence<
    mutable_buffer>
    : std::true_type
{
};

template<class T>
struct is_const_buffer_sequence<T, std::void_t<
    typename std::enable_if<
        (std::is_same<const_buffer, typename 
            T::value_type>::value
        || std::is_same<mutable_buffer, typename
            T::value_type>::value
            ) &&
        detail::is_bidirectional_iterator<typename
            T::const_iterator>::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().begin())
                >::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().end())
                >::value && (
        std::is_same<const_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value ||
        std::is_same<mutable_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value)
        // VFALCO This causes problems when the
        // trait is used to constrain ctors
        // && std::is_move_constructible<T>::value
            >::type
    > > : std::true_type
{
};

#endif

/** Determine if T is a MutableBuffers.
*/
#if BOOST_BUFFERS_DOCS
template<class T>
struct is_mutable_buffer_sequence
    : std::integral_constant<bool, ...>{};
#else

template<class T, class = void>
struct is_mutable_buffer_sequence : std::false_type
{
};

template<class T>
struct is_mutable_buffer_sequence<T const>
    : is_mutable_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_mutable_buffer_sequence<T const&>
    : is_mutable_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<class T>
struct is_mutable_buffer_sequence<T&>
    : is_mutable_buffer_sequence<typename
        std::decay<T>::type>
{
};

template<>
struct is_mutable_buffer_sequence<
    mutable_buffer>
    : std::true_type
{
};

template<class T>
struct is_mutable_buffer_sequence<T, std::void_t<
    typename std::enable_if<
        std::is_same<mutable_buffer, typename
            T::value_type>::value &&
        detail::is_bidirectional_iterator<typename
            T::const_iterator>::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().begin())
                >::value &&
        std::is_same<typename
            T::const_iterator, decltype(
            std::declval<T const&>().end())
                >::value &&
        std::is_same<mutable_buffer, typename
            std::remove_const<typename
                std::iterator_traits<
                    typename T::const_iterator
                        >::value_type>::type
                >::value
        // VFALCO This causes problems when the
        // trait is used to constrain ctors
        // && std::is_move_constructible<T>::value
            >::type
    >> : std::true_type
{
};

#endif

//------------------------------------------------

/** Determine if T is a DynamicBuffer
*/
#if BOOST_BUFFERS_DOCS
template<class T>
struct is_dynamic_buffer
    : std::integral_constant<bool, ...>{};
#else

template<
    class T,
    class = void>
struct is_dynamic_buffer : std::false_type {};

template<class T>
struct is_dynamic_buffer<
    T, std::void_t<decltype(
        std::declval<std::size_t&>() =
            std::declval<T const&>().size()
        ,std::declval<std::size_t&>() =
            std::declval<T const&>().max_size()
        ,std::declval<std::size_t&>() =
            std::declval<T const&>().capacity()
        ,std::declval<T&>().commit(
            std::declval<std::size_t>())
        ,std::declval<T&>().consume(
            std::declval<std::size_t>())
    )
    ,typename std::enable_if<
        is_const_buffer_sequence<typename
            T::const_buffers_type>::value
        && is_mutable_buffer_sequence<typename
            T::mutable_buffers_type>::value
        >::type
    ,typename std::enable_if<
        std::is_same<decltype(
            std::declval<T const&>().data()),
            typename T::const_buffers_type>::value
        && std::is_same<decltype(
            std::declval<T&>().prepare(
                std::declval<std::size_t>())),
            typename T::mutable_buffers_type>::value
        >::type
    > > : std::true_type
{
};

/** Return the underlying buffer type of a sequence.
*/
template<class T>
using value_type = typename
    std::conditional<
        is_mutable_buffer_sequence<T>::value,
        mutable_buffer,
        const_buffer
            >::type;

#endif

} // boost::buffers

#endif
