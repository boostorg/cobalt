//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_CONST_BUFFER_SPAN_HPP
#define BOOST_BUFFERS_CONST_BUFFER_SPAN_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/const_buffer_subspan.hpp>
#include <boost/async/io/buffers/type_traits.hpp>

namespace boost::async::io::buffers {

/** Holds a span of buffers that cannot be modified.

    Objects of this type meet the requirements
    of <em>ConstBufferSequence</em>.
*/
class const_buffer_span
{
    const_buffer const* p_ = nullptr;
    std::size_t n_ = 0;

    friend class const_buffer_subspan;

public:
    /** The type of buffer.
    */
    using value_type = const_buffer;

    /** The type of iterators returned.
    */
    using const_iterator = value_type const*;

    /** Constructor.
    */
    const_buffer_span() = default;

    /** Constructor.
    */
    const_buffer_span(
        const_buffer const* p,
        std::size_t n) noexcept
        : p_(p)
        , n_(n)
    {
    }

    /** Constructor.
    */
    template<const_buffer_sequence ConstBufferSequence>
        requires requires (const ConstBufferSequence & seq) {{seq.begin()} -> std::same_as<const const_buffer*>;}
    explicit
    const_buffer_span(
        ConstBufferSequence const& bs) noexcept
        : p_(bs.begin())
        , n_(bs.end() - bs.begin())
    {
    }

    /** Constructor.
    */
    const_buffer_span(
        const_buffer_span const&) = default;

    /** Assignment.
    */
    const_buffer_span& operator=(
        const_buffer_span const&) = default;

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
    const_buffer_subspan
    tag_invoke(
        prefix_tag const&,
        const_buffer_span const& s,
        std::size_t n) noexcept
    {
        return s.prefix_impl(n);
    }

    friend
    const_buffer_subspan
    tag_invoke(
        suffix_tag const&,
        const_buffer_span const& s,
        std::size_t n) noexcept
    {
        return s.suffix_impl(n);
    }
#endif

private:
    const_buffer_subspan prefix_impl(
        std::size_t n) const noexcept;
    const_buffer_subspan suffix_impl(
        std::size_t n) const noexcept;
};

//-----------------------------------------------

} // boost::buffers

#include <boost/async/io/buffers/impl/const_buffer_span.hpp>

#endif
