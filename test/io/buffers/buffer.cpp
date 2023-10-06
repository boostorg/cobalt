//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/buffer.hpp>

#include "test_suite.hpp"

namespace boost::async::io::buffers {

struct buffer_test
{
    void
    testBuffer()
    {
        char buf[3]{};
        char const* cbuf = buf;

        // buffer(mutable_buffer)
        {
            mutable_buffer b(buf, 3);
            auto b1 = buffer(b);
            BOOST_TEST_EQ(b1.data(), b.data());
            BOOST_TEST_EQ(b1.size(), b.size());
        }

        // buffer(void*, std::size_t)
        {
            auto b = buffer(buf, 3);
            BOOST_TEST_EQ(b.data(), buf);
            BOOST_TEST_EQ(b.size(), 3);
        }

        // buffer(const_buffer)
        {
            const_buffer b(cbuf, 3);
            auto b1 = buffer(b);
            BOOST_TEST_EQ(b1.data(), b.data());
            BOOST_TEST_EQ(b1.size(), b.size());
        }

        // buffer(void const*, std::size_t)
        {
            auto b = buffer(cbuf, 3);
            BOOST_TEST_EQ(b.data(), cbuf);
            BOOST_TEST_EQ(b.size(), 3);
        }

        // buffer(T(&)[N])
        {
            mutable_buffer b = buffer(buf);
            BOOST_TEST_EQ(b.data(), buf);
            BOOST_TEST_EQ(b.size(), 3);
        }

        // buffer(T const(&)[N])
        {
            char const cbuf3[3]{};
            const_buffer b = buffer(cbuf3);
            BOOST_TEST_EQ(b.data(), cbuf3);
            BOOST_TEST_EQ(b.size(), 3);
        }
    }

    void
    run()
    {
        testBuffer();
    }
};

TEST_SUITE(
    buffer_test,
    "boost.buffers.buffer");

} // boost::buffers
