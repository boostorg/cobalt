//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_MUTABLE_BUFFER_PAIR_HPP
#define BOOST_ASYNC_IO_BUFFERS_MUTABLE_BUFFER_PAIR_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/assert.hpp>

namespace boost::async::io::buffers {

/** A mutable buffer pair
*/
class mutable_buffer_pair
{
public:
    using value_type = mutable_buffer;

    using const_iterator = value_type const*;

    /** Constructor.
    */
    mutable_buffer_pair() = default;

    /** Constructor.
    */
    mutable_buffer_pair(
        mutable_buffer_pair const&) = default;

    /** Constructor.
    */
    mutable_buffer_pair(
        mutable_buffer const& b0,
        mutable_buffer const& b1) noexcept
        : b_{ b0, b1 }
    {
    }

    /** Assignment.
    */
    mutable_buffer_pair& operator=(
        mutable_buffer_pair const&) = default;

    mutable_buffer const&
    operator[](unsigned i) const noexcept
    {
        BOOST_ASSERT(i < 2);
        return b_[i];
    }

    mutable_buffer&
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

          mutable_buffer * data()       { return b_; }
    const mutable_buffer * data() const { return b_; }
    std::size_t size() const {return 2u;}

#ifndef BOOST_ASYNC_IO_BUFFERS_DOCS
    friend
    mutable_buffer_pair
    tag_invoke(
        prefix_tag const&,
        mutable_buffer_pair const& b,
        std::size_t n) noexcept
    {
        return b.prefix_impl(n);
    }

    friend
    mutable_buffer_pair
    tag_invoke(
        suffix_tag const&,
        mutable_buffer_pair const& b,
        std::size_t n) noexcept
    {
        return b.suffix_impl(n);
    }
#endif

private:
    BOOST_ASYNC_DECL
    mutable_buffer_pair
    prefix_impl(
        std::size_t n) const noexcept;

    BOOST_ASYNC_DECL
    mutable_buffer_pair
    suffix_impl(
        std::size_t n) const noexcept;

    mutable_buffer b_[2];
};

} // boost::buffers

#endif
