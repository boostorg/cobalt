//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_ALGORITHM_HPP
#define BOOST_BUFFERS_ALGORITHM_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/buffer_size.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <boost/async/io/buffers/tag_invoke.hpp>
#include <boost/async/io/buffers/type_traits.hpp>
#include <memory>

namespace boost::async::io::buffers {

#ifdef BOOST_BUFFERS_DOCS

/** Returns the type of a prefix of a buffer sequence.
*/
template<class BufferSequence>
using prefix_type = __see_below__;

/** Returns the type of a suffix of a buffer sequence.
*/
template<class BufferSequence>
using suffix_type = __see_below__;

/** Return a prefix of the buffer sequence.
*/
template<class BufferSequence>
prefix_type<BufferSequence>
prefix(
    BufferSequence const& b,
    std::size_t n);

/** Return a prefix of the buffer sequence.
*/
template<class BufferSequence>
prefix_type<BufferSequence>
sans_suffix(
    BufferSequence const& b,
    std::size_t n);

/** Return a suffix of the buffer sequence.
*/
template<class BufferSequence>
suffix_type<BufferSequence>
suffix(
    BufferSequence const& b,
    std::size_t n);

/** Return a suffix of the buffer sequence.
*/
template<class BufferSequence>
suffix_type<BufferSequence>
sans_prefix(
    BufferSequence const& b,
    std::size_t n);

/** Return the first buffer in a sequence.
*/
template<class BufferSequence>
__see_below__
front(
    BufferSequence const& b);

#else

template<class BufferSequence>
void
tag_invoke(
    prefix_tag const&,
    BufferSequence const&,
    std::size_t) = delete;

template<class BufferSequence>
void
tag_invoke(
    suffix_tag const&,
    BufferSequence const&,
    std::size_t) = delete;

template<class BufferSequence>
using prefix_type = decltype(
    tag_invoke(
        prefix_tag{},
        std::declval<BufferSequence const&>(),
        std::size_t{}));

template<class BufferSequence>
using suffix_type = decltype(
    tag_invoke(
        suffix_tag{},
        std::declval<BufferSequence const&>(),
        std::size_t{}));

namespace detail {

struct prefix_impl
{
    template<const_buffer_sequence BufferSequence>
    prefix_type<BufferSequence>
    operator()(
        BufferSequence const& b,
        std::size_t n) const
    {
        return tag_invoke(
            prefix_tag{}, b, n);
    }
};

struct sans_suffix_impl
{
    template<class BufferSequence>
    prefix_type<BufferSequence>
    operator()(
        BufferSequence const& b,
        std::size_t n) const
    {
        auto const n0 = buffer_size(b);
        if(n < n0)
            return tag_invoke(
                prefix_tag{}, b, n0 - n);
        return tag_invoke(
            prefix_tag{}, b, 0);
    }
};

struct suffix_impl
{
    template<const_buffer_sequence BufferSequence>
    suffix_type<BufferSequence>
    operator()(
        BufferSequence const& b,
        std::size_t n) const
    {
        return tag_invoke(
            suffix_tag{}, b, n);
    }
};

struct sans_prefix_impl
{
    template<const_buffer_sequence BufferSequence>
    suffix_type<BufferSequence>
    operator()(
        BufferSequence const& b,
        std::size_t n) const
    {
        auto const n0 = buffer_size(b);
        if(n < n0)
            return tag_invoke(
                suffix_tag{}, b, n0 - n);
        return tag_invoke(
            suffix_tag{}, b, 0);
    }
};

struct front_impl
{
    template<mutable_buffer_sequence MutableBufferSequence>
    mutable_buffer
    operator()(
        MutableBufferSequence const& bs) const noexcept
    {
        auto const it = begin(bs);
        if(it != end(bs))
            return *it;
        return {};
    }

    template<const_buffer_sequence ConstBufferSequence>
      requires (!mutable_buffer_sequence<ConstBufferSequence>)
    const_buffer
    operator()(
        ConstBufferSequence const& bs) const noexcept
    {
        auto const it = bs.begin();
        if(it != bs.end())
            return *it;
        return {};
    }
};

} // detail

constexpr detail::prefix_impl prefix{};
constexpr detail::suffix_impl suffix{};
constexpr detail::sans_prefix_impl sans_prefix{};
constexpr detail::sans_suffix_impl sans_suffix{};
constexpr detail::front_impl front{};

#endif

} // boost::buffers

#endif
