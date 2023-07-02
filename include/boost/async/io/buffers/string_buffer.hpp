//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_STRING_BUFFER_HPP
#define BOOST_BUFFERS_STRING_BUFFER_HPP

#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/assert.hpp>
#include <string>

namespace boost::async::io::buffers {

/** A dynamic buffer using an underlying string
*/
template<
    class CharT,
    class Traits = std::char_traits<CharT>,
    class Allocator = std::allocator<CharT>>
class basic_string_buffer
{
    std::basic_string<
        CharT, Traits, Allocator>* s_;
    std::size_t max_size_;

public:
    using string_type = std::basic_string<
        CharT, Traits, Allocator>;

    unsigned char* data_ = nullptr;
    std::size_t in_size_ = 0;
    std::size_t out_size_ = 0;

public:
    using const_buffers_type =
        const_buffer;

    using mutable_buffers_type =
        mutable_buffer;

    ~basic_string_buffer()
    {
        if(s_)
            s_->resize(in_size_);
    }

    /** Constructor.
    */
    basic_string_buffer(
        basic_string_buffer&& other) noexcept
        : s_(other.s_)
        , max_size_(other.max_size_)
    {
        other.s_ = nullptr;
    }

    /** Constructor.
    */
    explicit
    basic_string_buffer(
        string_type* s,
        std::size_t max_size =
            std::size_t(-1)) noexcept
        : s_(s)
        , max_size_(
            max_size > s_->max_size()
                ? s_->max_size()
                : max_size)
    {
        if(s_->size() > max_size_)
            s_->resize(max_size_);
        in_size_ = s_->size();
    }

    /** Assignment.
    */
    basic_string_buffer& operator=(
        basic_string_buffer const&) = delete;

    std::size_t
    size() const noexcept
    {
        return in_size_* sizeof(CharT);
    }

    std::size_t
    max_size() const noexcept
    {
        return max_size_ * sizeof(CharT);
    }

    std::size_t
    capacity() const noexcept
    {
        if(s_->capacity() <= max_size_)
            return s_->capacity() - in_size_;
        return (max_size_ - in_size_) * sizeof(CharT);
    }

    const_buffers_type
    data() const noexcept
    {
        return {
            s_->data(),
            in_size_ * sizeof(CharT)};
    }

    mutable_buffers_type
    prepare(std::size_t n_)
    {
        const auto n = (n_ / sizeof(CharT)) + (std::min)(n_ % sizeof(CharT), static_cast<std::size_t>(1u));

        // n exceeds available space
        if(n > max_size_ - in_size_)
            async::detail::throw_invalid_argument();

        if( s_->size() < in_size_ + n)
            s_->resize(in_size_ + n);
        out_size_ = n;
        return {
            &(*s_)[in_size_],
            out_size_ * sizeof(CharT)};
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
            s_->erase(0, n);
            in_size_ -= n;
        }
        else
        {
            in_size_ = 0;
        }
        out_size_ = 0;
    }
};

using string_buffer = basic_string_buffer<char>;

} // boost::buffers

#endif
