//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#include <boost/async/io/buffers/mutable_buffer_subspan.hpp>
#include <boost/async/io/buffers/algorithm.hpp>
#include <boost/assert.hpp>

namespace boost::async::io::buffers {

auto
mutable_buffer_subspan::
const_iterator::
operator*() const noexcept ->
    reference
{
    BOOST_ASSERT(s_->n_ > 0);
    auto b = s_->p_[i_];
    if(s_->n_ > 1)
    {
        if(i_ > 0)
        {
            if(i_ < s_->n_ - 1)
                return b;
            return prefix(b, s_->p1_);
        }
        return sans_prefix(b, s_->p0_);
    }
    return { static_cast<
        unsigned char*>(
            b.data()) + s_->p0_,
        s_->p1_ - s_->p0_ };
}

mutable_buffer_subspan::
mutable_buffer_subspan(
    mutable_buffer const* p,
    std::size_t n) noexcept
    : p_(p)
    , n_(n)
    , p1_([&]() -> std::size_t
        {
            if(n > 0)
                return p[n-1].size();
            return 0;
        }())
{
}

mutable_buffer_subspan
mutable_buffer_subspan::
prefix_impl(
    std::size_t n) const noexcept
{
    switch(n_)
    {
    case 0:
    {
        return *this;
    }
    case 1:
    {
        if(n == 0)
            return { p_, 0, p0_, p0_ };
        if(n == std::size_t(-1))
            return *this;
        auto const d = p1_ - p0_;
        if(n <= d)
            return { p_, 1, p0_, p0_ + n };
        return *this;
    }
    default:
    {
        if(n == 0)
            return { p_, 0, p0_, p0_ };
        if(n == std::size_t(-1))
            return *this;
        auto d = p_[0].size() - p0_;
        if(n <= d)
            return { p_, 1, p0_, p0_ + n };
        n -= d;
        std::size_t i = 1;
        for(;;)
        {
            if(i == n_ - 1)
                break;
            if(n <= p_[i].size())
                return { p_, i + 1, p0_, n };
            n -= p_[i].size();
            ++i;
        }
        if(n <= p1_)
            return { p_, n_, p0_, n };
        return { p_, n_, p0_, p1_ };
    }
    }
}

mutable_buffer_subspan
mutable_buffer_subspan::
suffix_impl(
    std::size_t n) const noexcept
{
    switch(n_)
    {
    case 0:
    {
        return *this;
    }
    case 1:
    {
        if(n == 0)
            return { p_, 0, p1_, p1_ };
        if(n == std::size_t(-1))
            return *this;
        auto const d = p1_ - p0_;
        if(n < d)
            return { p_, 1, p1_ - n, p1_ };
        return *this;
    }
    default:
    {
        if(n == 0)
            return { p_, 0, p1_, p1_ };
        if(n == std::size_t(-1))
            return *this;
        std::size_t i = n_ - 1;
        if(n <= p1_)
            return { p_ + i, 1, p1_ - n, p1_ };
        n -= p1_;
        for(;;)
        {
            if(--i == 0)
                break;
            if(n <= p_[i].size())
                return { p_ + i, n_ - i,
                    p_[i].size() - n, p1_ };
            n -= p_[i].size();
        }
        auto d = p_[0].size() - p0_;
        if(n <= d)
            return { p_, n_,
                p_[0].size() - n, p1_ };
        return { p_, n_, p0_, p1_ };
    }
    }
}

} // boost::buffers

