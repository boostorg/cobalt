//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/range.hpp>

#include "test_suite.hpp"

namespace boost::async::io::buffers {

struct range_test
{
    void
    run()
    {
    }
};

TEST_SUITE(
    range_test,
    "boost.buffers.range");

} // boost::buffers
