//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_CONST_BUFFER_PAIR_HPP
#define BOOST_BUFFERS_CONST_BUFFER_PAIR_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer_pair.hpp>
#include <boost/assert.hpp>

namespace boost::async::io::buffers {

/** A constant buffer pair
*/
class const_buffer_pair
{
public:
    using value_type = const_buffer;

    using const_iterator = value_type const*;

    /** Constructor.
    */
    const_buffer_pair() = default;

    /** Constructor.
    */
    const_buffer_pair(
        const_buffer_pair const&) = default;

    /** Constructor.
    */
    const_buffer_pair(
        const_buffer const& b0,
        const_buffer const& b1) noexcept
        : b_{ b0, b1 }
    {
    }

    /** Constructor.
    */
    const_buffer_pair(
        mutable_buffer_pair const& bs) noexcept
        : b_{ bs.begin()[0], bs.begin()[1] }
    {
    }

    /** Assignment.
    */
    const_buffer_pair& operator=(
        const_buffer_pair const&) = default;

    /** Assignment.
    */
    const_buffer_pair& operator=(
        mutable_buffer_pair const& other) noexcept
    {
        b_[0] = other.begin()[0];
        b_[1] = other.begin()[1];
        return *this;
    }

    const_buffer const&
    operator[](unsigned i) const noexcept
    {
        BOOST_ASSERT(i < 2);
        return b_[i];
    }

    const_buffer&
    operator[](unsigned i) noexcept
    {
        BOOST_ASSERT(i < 2);
        return b_[i];
    }

    const_iterator
    begin() const noexcept
    {
        return b_;
    }

    const_iterator
    end() const noexcept
    {
        return b_ + 2;
    }

#ifndef BOOST_BUFFERS_DOCS
    friend
    const_buffer_pair
    tag_invoke(
        prefix_tag const&,
        const_buffer_pair const& b,
        std::size_t n) noexcept
    {
        return b.prefix_impl(n);
    }

    friend
    const_buffer_pair
    tag_invoke(
        suffix_tag const&,
        const_buffer_pair const& b,
        std::size_t n) noexcept
    {
        return b.suffix_impl(n);
    }
#endif

private:
    BOOST_ASYNC_DECL
    const_buffer_pair
    prefix_impl(
        std::size_t n) const noexcept;

    BOOST_ASYNC_DECL
    const_buffer_pair
    suffix_impl(
        std::size_t n) const noexcept;

    const_buffer b_[2];
};

} // boost::buffers

#endif
