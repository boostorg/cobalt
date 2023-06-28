//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_IMPL_MUTABLE_BUFFER_SUBSPAN_HPP
#define BOOST_BUFFERS_IMPL_MUTABLE_BUFFER_SUBSPAN_HPP

#include <boost/assert.hpp>
#include <iterator>

namespace boost::async::io::buffers {

class mutable_buffer_subspan::
    const_iterator
{
    mutable_buffer_subspan const* s_ = nullptr;
    std::size_t i_ = 0;

    friend class mutable_buffer_subspan;

    const_iterator(
        mutable_buffer_subspan const& s,
        std::size_t i) noexcept
        : s_(&s)
        , i_(i)
    {
    }

public:
    using value_type = mutable_buffer;
    using reference = mutable_buffer;
    using pointer = void;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    const_iterator() = default;
    const_iterator(
        const_iterator const&) = default;
    const_iterator& operator=(
        const_iterator const&) = default;

    bool
    operator==(
        const_iterator const& other) const noexcept
    {
        return
            s_ == other.s_ &&
            i_ == other.i_;
    }

    bool
    operator!=(
        const_iterator const& other) const noexcept
    {
        return !(*this == other);
    }

    BOOST_ASYNC_DECL
    reference
    operator*() const noexcept;

    const_iterator&
    operator++() noexcept
    {
        BOOST_ASSERT(i_ < s_->n_);
        ++i_;
        return *this;
    }

    const_iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    const_iterator&
    operator--() noexcept
    {
        BOOST_ASSERT(i_ > 0);
        --i_;
        return *this;
    }

    const_iterator
    operator--(int) noexcept
    {
        auto temp = *this;
        --(*this);
        return temp;
    }
};

inline
auto
mutable_buffer_subspan::
begin() const noexcept ->
    const_iterator
{
    return { *this, 0 };
}

inline
auto
mutable_buffer_subspan::
end() const noexcept ->
    const_iterator
{
    return { *this, n_ };
}

inline
mutable_buffer_subspan::
mutable_buffer_subspan(
    mutable_buffer const* p,
    std::size_t n,
    std::size_t p0,
    std::size_t p1) noexcept
    : p_(p)
    , n_(n)
    , p0_(p0)
    , p1_(p1)
{
    BOOST_ASSERT(
        n_ > 1 ||
        p1_ >= p0_);
    BOOST_ASSERT(
        n_ == 0 ||
        p0 < p[0].size());
    BOOST_ASSERT(
        n_ == 0 ||
        p1 <= p[n_ - 1].size());
}

} // boost::buffers

#endif
