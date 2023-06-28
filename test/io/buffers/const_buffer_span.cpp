//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/const_buffer_span.hpp>

#include <boost/async/io/buffers/const_buffer_pair.hpp>
#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct const_buffer_span_test
{
    void
    testMembers()
    {
        auto const& pat = test_pattern();
        const_buffer cb[3] = {
            { &pat[0], 3 },
            { &pat[3], 5 },
            { &pat[8], 7 } };

        // const_buffer_span()
        {
            const_buffer_span bs;
            BOOST_TEST_EQ(buffer_size(bs), 0);
        }

        // const_buffer_span(
        //  const_buffer const*,
        //  std::size_t n)
        {
            const_buffer_span cbs(cb, 3);
            test_buffer_sequence(cbs);
        }

        // const_buffer_span(
        //  ConstBufferSequence
        {
            const_buffer_pair bp;
            const_buffer_span sp(bp);
            BOOST_TEST(
                sp.end() - sp.begin() == 2);
        }

        // const_buffer_span(
        //  const_buffer_span const&)
        {
            const_buffer_span cbs0(cb, 3);
            const_buffer_span cbs1(cbs0);
            test_buffer_sequence(cbs1);
        }

        // operator=(
        //  const_buffer_span const&)
        {
            const_buffer_span cbs0(cb, 3);
            const_buffer_span cbs1;
            cbs1 = cbs0;
            test_buffer_sequence(cbs1);
        }
    }

    void
    run()
    {
        testMembers();
    }
};

TEST_SUITE(
    const_buffer_span_test,
    "boost.buffers.const_buffer_span");

} // boost::buffers
