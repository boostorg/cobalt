//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#include <boost/async/io/buffers/const_buffer_pair.hpp>

namespace boost::async::io::buffers {

const_buffer_pair
const_buffer_pair::
prefix_impl(
    std::size_t n) const noexcept
{
    auto const it0 = begin();
    if(n <= it0->size())
        return { {
            it0->data(), n },
            const_buffer{} };
    n -= it0->size();
    auto it1 = it0;
    ++it1;
    if(n < it1->size())
        return { *it0, {
            it1->data(), n } };
    return *this;
}

const_buffer_pair
const_buffer_pair::
suffix_impl(
    std::size_t n) const noexcept
{
    auto it0 = end();
    --it0;
    if(n <= it0->size())
        return { *it0 + (
                it0->size() - n),
            const_buffer{} };
    n -= it0->size();
    auto it1 = it0;
    --it1;
    if(n < it1->size())
        return { *it1 + (
            it1->size() - n), *it0 };
    return *this;
}

} // boost::buffers
