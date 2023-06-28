//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/mutable_buffer_pair.hpp>

#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct mutable_buffer_pair_test
{
    void
    testMembers()
    {
        std::string pat = test_pattern();

        // mutable_buffer_pair()
        {
            mutable_buffer_pair mb;
            BOOST_TEST_EQ(
                buffer_size(mb), 0);
        }

        // mutable_buffer_pair(
        //  mutable_buffer_pair const&),
        // mutable_buffer_pair(
        //  mutable_buffer const&)
        //  mutable_buffer const&)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                mutable_buffer_pair mb0(
                    { &pat[0], i },
                    { &pat[i],
                        pat.size() - i });
                mutable_buffer_pair mb1(mb0);
                BOOST_TEST_EQ(
                    test_to_string(mb0), pat);
                BOOST_TEST_EQ(
                    test_to_string(mb0),
                    test_to_string(mb1));
                BOOST_TEST_EQ(
                    mb0[0].data(), mb1[0].data());
                BOOST_TEST_EQ(
                    mb0[1].size(), mb1[1].size());
                auto const& cmb0 = mb0;
                auto const& cmb1 = mb1;
                BOOST_TEST_EQ(
                    cmb0[0].data(), cmb1[0].data());
                BOOST_TEST_EQ(
                    cmb0[1].size(), cmb1[1].size());
            }
        }

        // operator=(mutable_buffer_pair const&)
        {
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                mutable_buffer_pair mb0(
                    { &pat[0], i },
                    { &pat[i],
                        pat.size() - i });
                mutable_buffer_pair mb1;
                mb1 = mb0;
                BOOST_TEST_EQ(
                    test_to_string(mb0), pat);
                BOOST_TEST_EQ(
                    test_to_string(mb0),
                    test_to_string(mb1));
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
                mutable_buffer_pair mb;
                mb = b;
                BOOST_TEST_EQ(
                    test_to_string(mb), pat);
                BOOST_TEST_EQ(
                    test_to_string(mb),
                    test_to_string(b));
            }
        }
    }

    void
    testBuffer()
    {
        std::string pat = test_pattern();
        for(std::size_t i = 0;
            i <= pat.size(); ++i)
        {
            mutable_buffer_pair cb(
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
    mutable_buffer_pair_test,
    "boost.buffers.mutable_buffer_pair");

} // boost::buffers
