//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/const_buffer.hpp>

#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct const_buffer_test
{
    void
    testMembers()
    {
        // const_buffer()
        BOOST_TEST_EQ(const_buffer().size(), 0);

        // const_buffer(void const*, size_t)
        {
            auto p = "12345";
            const_buffer b( p, 5 );
            BOOST_TEST_EQ(b.data(), p);
            BOOST_TEST_EQ(b.size(), 5);
        }

        // const_buffer(const_buffer)
        {
            auto p = "12345";
            const_buffer b0( p, 5 );
            const_buffer b(b0);
            BOOST_TEST_EQ(b.data(), p);
            BOOST_TEST_EQ(b.size(), 5);
        }

        // const_buffer(mutable_buffer)
        {
            char buf[6] = "12345";
            mutable_buffer b0( buf, 5 );
            const_buffer b(b0);
            BOOST_TEST_EQ(b.data(), buf);
            BOOST_TEST_EQ(b.size(), 5);
        }

        // operator=(const_buffer)
        {
            auto p = "12345";
            const_buffer b;
            b = const_buffer(p, 5);
            BOOST_TEST_EQ(b.data(), p);
            BOOST_TEST_EQ(b.size(), 5);
        }

        // operator+=(std::size_t)
        {
            {
                auto p = "12345";
                const_buffer b;
                b = const_buffer(p, 5);
                b += 2;
                BOOST_TEST_EQ(b.data(), p + 2);
                BOOST_TEST_EQ(b.size(), 3);
            }
            {
                auto p = "12345";
                const_buffer b;
                b = const_buffer(p, 5);
                b += 6;
                BOOST_TEST_EQ(b.size(), 0);
            }
        }

        // operator+(const_buffer, size_t)
        // operator+(size_t, const_buffer)
        {
            auto p = "12345";
            const_buffer b0(p, 5);
            const_buffer b1(p, 5);
            b0 = b0 + 2;
            b1 = 2 + b1;
            BOOST_TEST_EQ(b0.data(), p + 2);
            BOOST_TEST_EQ(b0.size(), 3);
            BOOST_TEST_EQ(b0.data(), b1.data());
            BOOST_TEST_EQ(b0.size(), b1.size());
        }
    }

    void
    testBuffer()
    {
        auto const& pat = test_pattern();
        const_buffer cb(&pat[0], pat.size());
        test_buffer_sequence(cb);
    }

    void
    run()
    {
        testMembers();
        testBuffer();
    }
};

TEST_SUITE(
    const_buffer_test,
    "boost.buffers.const_buffer");

} // boost::buffers
