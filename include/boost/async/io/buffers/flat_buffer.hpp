//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_FLAT_BUFFER_HPP
#define BOOST_BUFFERS_FLAT_BUFFER_HPP

#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/assert.hpp>

namespace boost::async::io::buffers {

/** A DynamicBuffer with a fixed capacity
*/
class flat_buffer
{
    unsigned char* data_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t in_pos_ = 0;
    std::size_t in_size_ = 0;
    std::size_t out_size_ = 0;

public:
    using const_buffers_type =
        const_buffer;

    using mutable_buffers_type =
        mutable_buffer;

    /** Constructor.
    */
    flat_buffer() = default;

    /** Constructor.
    */
    flat_buffer(
        void* data,
        std::size_t capacity,
        std::size_t initial_size = 0)
        : data_(static_cast<
            unsigned char*>(data))
        , cap_(capacity)
        , in_size_(initial_size)
    {
        // initial size too large
        if(in_size_ > cap_)
            async::detail::throw_invalid_argument();
    }

    /** Constructor.
    */
    explicit
    flat_buffer(
        mutable_buffer const& b,
        std::size_t initial_size = 0)
        : flat_buffer(
            b.data(),
            b.size(),
            initial_size)
    {
    }

    /** Constructor.
    */
    flat_buffer(
        flat_buffer const&) = default;

    /** Constructor.
    */
    flat_buffer& operator=(
        flat_buffer const&) = default;

    std::size_t
    size() const noexcept
    {
        return in_size_;
    }

    std::size_t
    max_size() const noexcept
    {
        return cap_;
    }

    std::size_t
    capacity() const noexcept
    {
        return cap_ - in_pos_;
    }

    const_buffers_type
    data() const noexcept
    {
        return {
            data_ + in_pos_,
            in_size_ };
    }

    mutable_buffers_type
    prepare(std::size_t n)
    {
        // n exceeds available space
        if(n > cap_ - in_size_)
            async::detail::throw_invalid_argument();

        out_size_ = n;
        return { data_ +
            in_pos_ + in_size_, n };
    }

    void
    commit(
        std::size_t n) noexcept
    {
        if(n < out_size_)
            in_size_ += n;
        else
            in_size_ += out_size_;
        out_size_ = 0;
    }

    void
    consume(
        std::size_t n) noexcept
    {
        if(n < in_size_)
        {
            in_pos_ += n;
            in_size_ -= n;
        }
        else
        {
            in_pos_ = 0;
            in_size_ = 0;
        }
    }
};

} // boost::buffers

#endif
