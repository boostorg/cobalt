//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_IMPL_CONST_BUFFER_SPAN_HPP
#define BOOST_BUFFERS_IMPL_CONST_BUFFER_SPAN_HPP

namespace boost::async::io::buffers {

inline
const_buffer_subspan
const_buffer_span::
prefix_impl(
    std::size_t n) const noexcept
{
    return const_buffer_subspan(
        *this).prefix_impl(n);
}

inline
const_buffer_subspan
const_buffer_span::
suffix_impl(
    std::size_t n) const noexcept
{
    return const_buffer_subspan(
        *this).suffix_impl(n);
}

//-----------------------------------------------

// here because circular dependency
inline
const_buffer_subspan::
const_buffer_subspan(
    const_buffer_span const& s) noexcept
    : p_(s.p_)
    , n_(s.n_)
    , p1_([&]() -> std::size_t
        {
            if(n_ > 0)
                return p_[n_-1].size();
            return 0;
        }())
{
}


} // boost::buffers

#endif
