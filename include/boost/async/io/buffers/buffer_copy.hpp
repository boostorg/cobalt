//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_BUFFER_COPY_HPP
#define BOOST_ASYNC_IO_BUFFERS_BUFFER_COPY_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <boost/async/io/buffers/concepts.hpp>
#include <boost/assert.hpp>
#include <cstring>
#include <utility>

namespace boost::async::io::buffers {

#ifdef BOOST_ASYNC_IO_BUFFERS_DOCS

/** Copy buffer contents
*/
template<
    class MutableBufferSequence,
    class ConstBufferSequence>
std::size_t
buffer_copy(
    MutableBufferSequence const& to,
    ConstBufferSequence const& from,
    std::size_t at_most =
        std::size_t(-1)) noexcept;

#else

namespace detail {

struct buffer_copy_impl
{
    // If you get a compile error here it
    // means that one or both of your types
    // do not meet the requirements.
    template<
        mutable_buffer_sequence MutableBuffers,
        const_buffer_sequence ConstBuffers>
    std::size_t
    operator()(
        MutableBuffers const& to,
        ConstBuffers const& from,
        std::size_t at_most =
            std::size_t(-1)) const noexcept
    {
        std::size_t total = 0;
        std::size_t pos0 = 0;
        std::size_t pos1 = 0;
        auto const end0 = end(from);
        auto const end1 = end(to);
        auto it0 = begin(from);
        auto it1 = begin(to);
        while(
            total < at_most &&
            it0 != end0 &&
            it1 != end1)
        {
            const_buffer b0 =
                const_buffer(*it0) + pos0;
            mutable_buffer b1 =
                mutable_buffer(*it1) + pos1;
            std::size_t const amount =
            [&]
            {
                std::size_t n = b0.size();
                if( n > b1.size())
                    n = b1.size();
                if( n > at_most - total)
                    n = at_most - total;
                std::memcpy(
                    b1.data(),
                    b0.data(),
                    n);
                return n;
            }();
            total += amount;
            if(amount == b1.size())
            {
                ++it1;
                pos1 = 0;
            }
            else
            {
                pos1 += amount;
            }
            if(amount == b0.size())
            {
                ++it0;
                pos0 = 0;
            }
            else
            {
                pos0 += amount;
            }
        }
        return total;
    }
};

} // detail

constexpr detail::buffer_copy_impl buffer_copy{};

#endif

} // boost::buffers

#endif
