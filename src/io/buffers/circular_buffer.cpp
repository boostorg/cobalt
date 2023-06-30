//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#include <boost/async/io/buffers/circular_buffer.hpp>
#include <boost/async/io/buffers/type_traits.hpp>
#include <boost/async/detail/exception.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

namespace boost::async::io::buffers {

BOOST_STATIC_ASSERT(dynamic_buffer<circular_buffer>);

auto
circular_buffer::
data() const noexcept ->
    const_buffers_type
{
    if(in_pos_ + in_len_ <= cap_)
        return {
            const_buffer{
                base_ + in_pos_, in_len_ },
            const_buffer{ base_, 0} };
    return {
        const_buffer{
            base_ + in_pos_, cap_ - in_pos_},
        const_buffer{
            base_, in_len_- (cap_ - in_pos_)}};
}

auto
circular_buffer::
prepare(std::size_t n) ->
    mutable_buffers_type
{
    // Buffer is too small for n
    if(n > cap_ - in_len_)
        detail::throw_length_error();

    out_size_ = n;
    auto const pos = (
        in_pos_ + in_len_) % cap_;
    if(pos + n <= cap_)
        return {
            mutable_buffer{
                base_ + pos, n},
            mutable_buffer{base_, 0}};
    return {
        mutable_buffer{
            base_ + pos, cap_ - pos},
        mutable_buffer{
            base_, n - (cap_ - pos)}};
}

void
circular_buffer::
commit(
    std::size_t n) noexcept
{
    if(n < out_size_)
        in_len_ += n;
    else
        in_len_ += out_size_;
    out_size_ = 0;
}

void
circular_buffer::
consume(
    std::size_t n) noexcept
{
    if(n < in_len_)
    {
        in_pos_ = (in_pos_ + n) % cap_;
        in_len_ -= n;
    }
    else
    {
        // make prepare return a
        // bigger single buffer
        in_pos_ = 0;
        in_len_ = 0;
    }
}

} // boost::buffers
