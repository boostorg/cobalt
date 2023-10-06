//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/buffer_size.hpp>
#include <boost/async/io/buffers/const_buffer_span.hpp>

#include "test_suite.hpp"

namespace boost::async::io::buffers {

struct buffer_size_test
{
    void
    testBufferSize()
    {
        char data[9];
        for(std::size_t i = 0; i < 3; ++i)
        for(std::size_t j = 0; j < 3; ++j)
        for(std::size_t k = 0; k < 3; ++k)
        {
            const_buffer cb[3] = {
                { data, i },
                { data + i, j },
                { data + i + j, k }
            };
            const_buffer_span s(cb, 3);
            BOOST_TEST_EQ(
                buffer_size(s), i + j + k);
        }
    }

    void
    run()
    {
        testBufferSize();
    }
};

TEST_SUITE(
    buffer_size_test,
    "boost.buffers.buffer_size");

} // boost::buffers
