//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/buffer_copy.hpp>

#include <boost/async/io/buffers/const_buffer_span.hpp>
#include <boost/async/io/buffers/mutable_buffer_span.hpp>
#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct buffer_copy_test
{
    void
    testBufferCopy()
    {
        auto const& pat = test_pattern();

        for(std::size_t i = 0;
            i <= pat.size(); ++i)
        {
            for(std::size_t j = 0;
                j <= pat.size(); ++j)
            {
                std::string s;
                s.resize(pat.size());
                const_buffer cb[2] = {
                    { &pat[0], i },
                    { &pat[i],
                        pat.size() - i } };
                mutable_buffer mb[2] = {
                    { &s[0], j },
                    { &s[j],
                        pat.size() - j } };
                auto n = buffer_copy(
                    mutable_buffer_span(mb, 2),
                    const_buffer_span(cb, 2));
                BOOST_TEST_EQ(n, pat.size());
                BOOST_TEST_EQ(s, pat);
            }
            for(std::size_t j = 0;
                j <= pat.size(); ++j)
            {
                for(std::size_t k = 0;
                    k <= pat.size(); ++k)
                {
                    std::string s;
                    s.resize(pat.size());
                    const_buffer cb[2] = {
                        { &pat[0], i },
                        { &pat[i],
                            pat.size() - i } };
                    mutable_buffer mb[2] = {
                        { &s[0], j },
                        { &s[j],
                            pat.size() - j } };
                    auto n = buffer_copy(
                        mutable_buffer_span(mb, 2),
                        const_buffer_span(cb, 2), k);
                    s.resize(n);
                    BOOST_TEST_EQ(s,
                        pat.substr(0, k));
                }
            }
        }
    }
    void
    run()
    {
        testBufferCopy();
    }
};

TEST_SUITE(
    buffer_copy_test,
    "boost.buffers.buffer_copy");

} // boost::buffers
