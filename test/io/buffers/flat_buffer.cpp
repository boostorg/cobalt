//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/flat_buffer.hpp>

#include <boost/async/io/buffers/concepts.hpp>
#include <boost/static_assert.hpp>
#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct flat_buffer_test
{
    BOOST_STATIC_ASSERT(dynamic_buffer<flat_buffer>);

    void
    testMembers()
    {
        std::string pat = test_pattern();

        // flat_buffer()
        {
            flat_buffer fb;
            BOOST_TEST_EQ(fb.size(), 0);
            BOOST_TEST_EQ(fb.max_size(), 0);
            BOOST_TEST_EQ(fb.capacity(), 0);
        }

        // flat_buffer(void*, size_t)
        {
            std::string s = pat;
            flat_buffer fb(&s[0], s.size());
            BOOST_TEST_EQ(fb.size(), 0);
            BOOST_TEST_EQ(fb.max_size(), s.size());
            BOOST_TEST_EQ(fb.capacity(), s.size());
        }

        // flat_buffer(void*, size_t, size_t)
        {
            std::string s = pat;
            flat_buffer fb(&s[0], s.size(), 6);
            BOOST_TEST_EQ(fb.size(), 6);
            BOOST_TEST_EQ(fb.max_size(), s.size());
            BOOST_TEST_EQ(fb.capacity(), s.size());
        }
        {
            std::string s = pat;
            BOOST_TEST_THROWS(
                flat_buffer(&s[0], s.size(),
                    s.size() + 1),
                std::invalid_argument);
        }

        // flat_buffer(mutable_buffer)
        {
            std::string s = pat;
            flat_buffer fb(mutable_buffer(
                &s[0], s.size()));
            BOOST_TEST_EQ(fb.size(), 0);
            BOOST_TEST_EQ(fb.max_size(), s.size());
            BOOST_TEST_EQ(fb.capacity(), s.size());
        }

        // flat_buffer(mutable_buffer, size_t)
        {
            std::string s = pat;
            flat_buffer fb(mutable_buffer(
                &s[0], s.size()), 6);
            BOOST_TEST_EQ(fb.size(), 6);
            BOOST_TEST_EQ(fb.max_size(), s.size());
            BOOST_TEST_EQ(fb.capacity(), s.size());
        }

        // flat_buffer(flat_buffer const&)
        {
            std::string s = pat;
            flat_buffer fb0(&s[0], s.size());
            flat_buffer fb1(fb0);
            BOOST_TEST_EQ(fb1.size(), fb0.size());
            BOOST_TEST_EQ(fb1.max_size(), fb0.max_size());
            BOOST_TEST_EQ(fb1.capacity(), fb0.capacity());
        }

        // operator=(flat_buffer const&)
        {
            std::string s = pat;
            flat_buffer fb0(&s[0], s.size());
            flat_buffer fb1;
            fb1 = fb0;
            BOOST_TEST_EQ(fb1.size(), fb0.size());
            BOOST_TEST_EQ(fb1.max_size(), fb0.max_size());
            BOOST_TEST_EQ(fb1.capacity(), fb0.capacity());
        }

        // prepare(std::size_t)
        {
            std::string s = pat;
            flat_buffer fb(&s[0], s.size());
            BOOST_TEST_THROWS(
                fb.prepare(s.size() + 1),
                std::invalid_argument);
        }

        // commit(std::size_t)
        {
            std::string s = pat;
            for(std::size_t i = 0;
                i <= pat.size(); ++i)
            {
                flat_buffer fb(
                    &s[0], s.size());
                fb.prepare(s.size());
                fb.commit(i);
                BOOST_TEST_EQ(
                    test_to_string(fb.data()),
                    pat.substr(0, i));
            }
        }

        // consume(std::size_t)
        {
        }
    }

    void
    testBuffer()
    {
        auto const& pat = test_pattern();
        for(std::size_t i = 0;
            i <= pat.size(); ++i)
        {
            std::string s(pat.size(), 0);
            flat_buffer fb(&s[0], s.size());
            fb.commit(buffer_copy(
                fb.prepare(i),
                buffer(&pat[0], i)));
            fb.commit(buffer_copy(
                fb.prepare(pat.size() - i),
                buffer(&pat[i], pat.size() - i)));
            BOOST_TEST_EQ(test_to_string(
                fb.data()), pat);
            fb.consume(i);
            BOOST_TEST_EQ(test_to_string(
                fb.data()), pat.substr(i));
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
    flat_buffer_test,
    "boost.buffers.flat_buffer");

} // boost::buffers
