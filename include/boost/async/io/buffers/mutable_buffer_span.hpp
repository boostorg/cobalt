//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_MUTABLE_BUFFER_SPAN_HPP
#define BOOST_BUFFERS_MUTABLE_BUFFER_SPAN_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer_subspan.hpp>
#include <boost/async/io/buffers/type_traits.hpp>

namespace boost::async::io::buffers {

/** Holds a span of buffers that are modifiable.

    Objects of this type meet the requirements
    of <em>MutableBufferSequence</em>.
*/
class mutable_buffer_span
{
    mutable_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

    friend class mutable_buffer_subspan;

public:
    /** The type of buffer.
    */
    using value_type = mutable_buffer;

    /** The type of iterators returned.
    */
    using const_iterator = value_type const*;

    /** Constructor.
    */
    mutable_buffer_span() = default;

    /** Constructor.
    */
    mutable_buffer_span(
        mutable_buffer const* p,
        std::size_t n) noexcept
        : p_(p)
        , n_(n)
    {
    }

    /** Constructor.
    */
    template<mutable_buffer_sequence MutableBufferSequence>
        requires requires (const MutableBufferSequence & seq) {{seq.begin()} -> std::same_as<const mutable_buffer*>;}
    explicit
    mutable_buffer_span(
        MutableBufferSequence const& bs) noexcept
        : p_(bs.begin())
        , n_(bs.end() - bs.begin())
    {
    }

    /** Constructor.
    */
    mutable_buffer_span(
        mutable_buffer_span const&) = default;

    /** Assignment.
    */
    mutable_buffer_span& operator=(
        mutable_buffer_span const&) = default;

    /** Return an iterator to the beginning.
    */
    const_iterator
    begin() const noexcept
    {
        return p_;
    }

    /** Return an iterator to the end.
    */
    const_iterator
    end() const noexcept
    {
        return p_ + n_;
    }

#ifndef BOOST_BUFFERS_DOCS
    friend
    mutable_buffer_subspan
    tag_invoke(
        prefix_tag const&,
        mutable_buffer_span const& s,
        std::size_t n) noexcept
    {
        return s.prefix_impl(n);
    }

    friend
    mutable_buffer_subspan
    tag_invoke(
        suffix_tag const&,
        mutable_buffer_span const& s,
        std::size_t n) noexcept
    {
        return s.suffix_impl(n);
    }
#endif

private:
    mutable_buffer_subspan prefix_impl(
        std::size_t n) const noexcept;
    mutable_buffer_subspan suffix_impl(
        std::size_t n) const noexcept;
};

} // boost::buffers

#include <boost/async/io/buffers/impl/mutable_buffer_span.hpp>

#endif
