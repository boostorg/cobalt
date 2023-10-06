//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/any_dynamic_buffer.hpp>

#include <boost/async/io/buffers/circular_buffer.hpp>
#include <boost/static_assert.hpp>
#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct any_dynamic_buffer_test
{
    BOOST_STATIC_ASSERT(dynamic_buffer<any_dynamic_buffer>);

    void
    testAny()
    {
        auto const& pat = test_pattern();

        for(std::size_t i = 0;
            i <= pat.size(); ++i)
        for(std::size_t j = 0;
            j <=  pat.size(); ++j)
        for(std::size_t k = 0;
            k <= pat.size(); ++k)
        {
            std::string s(pat.size(), 0);
            auto db = make_any(circular_buffer(
                &s[0], s.size()));
            if( j < pat.size() &&
                i > 0)
            {
                db.prepare(i);
                db.commit(i);
                db.consume(i - 1);
                db.commit(buffer_copy(
                    db.prepare(j),
                    buffer(
                        pat.data(),
                        pat.size())));
                db.consume(1);
            }
            else
            {
                db.commit(buffer_copy(
                    db.prepare(j),
                    buffer(
                        pat.data(),
                        pat.size())));
            }
            db.commit(buffer_copy(
                db.prepare(pat.size() - j),
                buffer(
                    pat.data() + j,
                    pat.size() - j)));
            BOOST_TEST_EQ(test_to_string(
                db.data()), pat);
            test_buffer_sequence(db.data());
            db.consume(k);
            BOOST_TEST_EQ(test_to_string(
                db.data()), pat.substr(k));
        }
    }

    void
    run()
    {
        testAny();
    }
};

TEST_SUITE(
    any_dynamic_buffer_test,
    "boost.buffers.any_dynamic_buffer");

} // boost::buffers
