//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/const_buffer_subspan.hpp>

#include <boost/async/io/buffers/const_buffer_span.hpp>
#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct const_buffer_subspan_test
{
    void
    testMembers()
    {
        auto const& pat = test_pattern();
        const_buffer const cb[3] = {
            { pat.data(), 3 },
            { pat.data() + 3, 5 },
            { pat.data() + 8, 7 } };

        // const_buffer_subspan()
        {
            const_buffer_subspan bs;
            BOOST_TEST_EQ(buffer_size(bs), 0);
        }

        // const_buffer_subspan(
        //  const_buffer const*, std::size_t)
        {
            const_buffer_subspan s(cb, 3);
            BOOST_TEST_EQ(buffer_size(s), 15);
        }
        {
            const_buffer_subspan s(cb, 0);
            BOOST_TEST_EQ(buffer_size(s), 0);
        }

        // const_buffer_subspan(
        //  const_buffer_span const&)
        {
            const_buffer_span cs0(cb, 3);
            const_buffer_subspan cs1(cs0);
            BOOST_TEST_EQ(
                buffer_size(cs0),
                buffer_size(cs1));
        }
        {
            const_buffer_span cs0(cb, 0);
            const_buffer_subspan cs1(cs0);
            BOOST_TEST_EQ(
                buffer_size(cs0),
                buffer_size(cs1));
        }

        // const_buffer_subspan(
        //  const_buffer_subspan const&)
        {
            const_buffer_subspan s0(cb, 3);
            const_buffer_subspan s1(s0);
            BOOST_TEST_EQ(
                buffer_size(s1),
                buffer_size(s0));
        }

        // operator=(
        //  const_buffer_subspan const&)
        {
            const_buffer_subspan s;
            BOOST_TEST_EQ(buffer_size(s), 0);
            s = const_buffer_subspan(cb, 3);
            BOOST_TEST_EQ(buffer_size(s), 15);
        }
    }

    void
    testSequence()
    {
        auto const pat = test_pattern();

        // length 1
        {
            const_buffer cb = {
                &pat[0], pat.size() };
            const_buffer_subspan s(&cb, 1);
            test_buffer_sequence(s);
        }

        // length 2
        {
            const_buffer const cb[2] = {
                { &pat[0], 7 },
                { &pat[7], 8 } };
            const_buffer_subspan s(cb, 2);
            test_buffer_sequence(s);
        }

        // length 3
        {
            const_buffer const cb[3] = {
                { &pat[0], 3 },
                { &pat[3], 5 },
                { &pat[8], 7 } };
            const_buffer_subspan s(cb, 3);
            test_buffer_sequence(s);
        }
    }

    void
    testSubspan()
    {
        std::string tmp;
        auto const& pat = test_pattern();
        const_buffer const cb[3] = {
            { &pat[0], 3 },
            { &pat[3], 5 },
            { &pat[8], 7 } };
        const_buffer_span const cs0(cb, 3);

        for(std::size_t i = 0; i <= pat.size(); ++i )
        {
            {
                auto b0 = prefix(cs0, i);
                auto b1 = sans_prefix(cs0, i);
                tmp = std::string(pat.size(), ' ');
                mutable_buffer dest(&tmp[0], tmp.size());
                auto n = buffer_copy(dest, b0);
                dest += n;
                n += buffer_copy(dest, b1);
                BOOST_TEST_EQ(n, pat.size());
                BOOST_TEST_EQ(tmp, pat);
            }
            for(std::size_t j = 0; j <= pat.size(); ++j)
            {
                auto b = prefix(sans_prefix(cs0, i), j);
                tmp.resize(pat.size());
                tmp.resize(buffer_copy(
                    mutable_buffer(
                        &tmp[0], tmp.size()), b));
                if(i <= pat.size())
                    BOOST_TEST_EQ(tmp, pat.substr(i, j));
                else
                    BOOST_TEST(tmp.empty());
            }
            for(std::size_t j = 0; j <= pat.size(); ++j)
            {
                auto b = suffix(sans_suffix(cs0, i), j);
                tmp.resize(pat.size());
                tmp.resize(buffer_copy(
                    mutable_buffer(
                        &tmp[0], tmp.size()), b));
                if(i <= pat.size())
                {
                    auto n = pat.size() - i; // inner length
                    if(n >= j)
                        BOOST_TEST_EQ(tmp,
                            pat.substr(n - j, j));
                    else
                        BOOST_TEST_EQ(tmp,
                            pat.substr(0, n));
                }
                else
                {
                    BOOST_TEST(tmp.empty());
                }
            }
        }
    }

    void
    run()
    {
        testMembers();
        testSequence();
        testSubspan();
    }
};

TEST_SUITE(
    const_buffer_subspan_test,
    "boost.buffers.const_buffer_subspan");

} // boost::buffers
