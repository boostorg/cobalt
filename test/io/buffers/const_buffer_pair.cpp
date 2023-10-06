//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/const_buffer_pair.hpp>

#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct const_buffer_pair_test
{
    void
    testMembers()
    {
        auto const& pat = test_pattern();

        // const_buffer_pair()
        {
            const_buffer_pair cb;
            BOOST_TEST_EQ(
                buffer_size(cb), 0);
        }

        // const_buffer_pair(
        //  const_buffer_pair const&),
        // const_buffer_pair(
        //  const_buffer const&)
        //  const_buffer const&)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                const_buffer_pair cb0(
                    { &pat[0], i },
                    { &pat[i],
                        pat.size() - i });
                const_buffer_pair cb1(cb0);
                BOOST_TEST_EQ(
                    test_to_string(cb0), pat);
                BOOST_TEST_EQ(
                    test_to_string(cb0),
                    test_to_string(cb1));
                BOOST_TEST_EQ(
                    cb0[0].data(), cb1[0].data());
                BOOST_TEST_EQ(
                    cb0[1].size(), cb1[1].size());
                auto const& ccb0 = cb0;
                auto const& ccb1 = cb1;
                BOOST_TEST_EQ(
                    ccb0[0].data(), ccb1[0].data());
                BOOST_TEST_EQ(
                    ccb0[1].size(), ccb1[1].size());
            }
        }

        // const_buffer_pair(
        //  mutable_buffer_pair)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                auto s = pat;
                mutable_buffer_pair b(
                    { &s[0], i },
                    { &s[i],
                        s.size() - i });
                const_buffer_pair cb(b);
                BOOST_TEST_EQ(
                    test_to_string(cb), pat);
                BOOST_TEST_EQ(
                    test_to_string(cb),
                    test_to_string(b));
            }
        }

        // operator=(const_buffer_pair const&)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                const_buffer_pair cb0(
                    { &pat[0], i },
                    { &pat[i],
                        pat.size() - i });
                const_buffer_pair cb1;
                cb1 = cb0;
                BOOST_TEST_EQ(
                    test_to_string(cb0), pat);
                BOOST_TEST_EQ(
                    test_to_string(cb0),
                    test_to_string(cb1));
            }
        }

        // operator=(mutable_buffer_pair const&)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                auto s = pat;
                mutable_buffer_pair b(
                    { &s[0], i },
                    { &s[i],
                        s.size() - i });
                const_buffer_pair cb;
                cb = b;
                BOOST_TEST_EQ(
                    test_to_string(cb), pat);
                BOOST_TEST_EQ(
                    test_to_string(cb),
                    test_to_string(b));
            }
        }
    }

    void
    testBuffer()
    {
        auto const& pat = test_pattern();
        for(std::size_t i = 0;
            i <= pat.size(); ++i)
        {
            const_buffer_pair cb(
                { &pat[0], i },
                { &pat[i],
                    pat.size() - i });
            test_buffer_sequence(cb);
        }
    }

    void
    run()
    {
        testMembers();
        testBuffer();
    }
};

TEST_SUITE(
    const_buffer_pair_test,
    "boost.buffers.const_buffer_pair");

} // boost::buffers
