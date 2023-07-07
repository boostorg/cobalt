//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_CIRCULAR_BUFFER_HPP
#define BOOST_ASYNC_IO_BUFFERS_CIRCULAR_BUFFER_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer_pair.hpp>
#include <boost/async/io/buffers/mutable_buffer_pair.hpp>
#include <boost/async/detail/exception.hpp>

namespace boost::async::io::buffers {

class circular_buffer
{
    unsigned char* base_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t in_pos_ = 0;
    std::size_t in_len_ = 0;
    std::size_t out_size_ = 0;

public:
    using const_buffers_type =
        const_buffer_pair;

    using mutable_buffers_type =
        mutable_buffer_pair;

    /** Constructor.
    */
    circular_buffer() = default;

    /** Constructor.
    */
    circular_buffer(
        circular_buffer const&) = default;

#if 0
    /** Constructor.
    */
    circular_buffer(
        mutable_buffer b) noexcept
        : base_(static_cast<
            unsigned char*>(b.data()))
        , cap_(b.size())
    {
    }
#endif

    /** Constructor.
    */
    circular_buffer(
        void* base,
        std::size_t capacity) noexcept
        : base_(static_cast<
            unsigned char*>(base))
        , cap_(capacity)
    {
    }

    /** Constructor.
    */
    circular_buffer(
        void* base,
        std::size_t capacity,
        std::size_t initial_size)
        : base_(static_cast<
            unsigned char*>(base))
        , cap_(capacity)
        , in_len_(initial_size)
    {
        if(in_len_ > capacity)
            async::detail::throw_invalid_argument();
    }

    /** Assignment.
    */
    circular_buffer& operator=(
        circular_buffer const&) = default;

    std::size_t
    size() const noexcept
    {
        return in_len_;
    }

    std::size_t
    max_size() const noexcept
    {
        return cap_;
    }
    
    std::size_t
    capacity() const noexcept
    {
        return cap_ - in_len_;
    }

    BOOST_ASYNC_DECL
    const_buffers_type
    data() const noexcept;
    
    BOOST_ASYNC_DECL
    mutable_buffers_type
    prepare(std::size_t n);

    BOOST_ASYNC_DECL
    void
    commit(std::size_t n) noexcept;

    BOOST_ASYNC_DECL
    void
    consume(std::size_t n) noexcept;
};

} // boost::buffers

#endif
