//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/dynamic_buffer_view.hpp>

#include <boost/async/io/buffers/flat_buffer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/static_assert.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/container/devector.hpp>
#include <boost/container/deque.hpp>

#include "test_helpers.hpp"

namespace boost::async::io::buffers {

struct dynamic_buffer_view_test
{
    BOOST_STATIC_ASSERT(  const_buffer_sequence<dynamic_buffer_view::  const_buffers_type>);
    BOOST_STATIC_ASSERT(mutable_buffer_sequence<dynamic_buffer_view::mutable_buffers_type>);
    BOOST_STATIC_ASSERT(dynamic_buffer<dynamic_buffer_view>);

    void testOther()
    {
        char cc[64];
        flat_buffer fb{cc, 64};
        dynamic_buffer_view dbv{fb};

        std::string data;
        data.resize(64);
        char c = ' ';
        std::generate(data.begin(), data.end(), [&]{return c++;});

        BOOST_TEST_EQ(dbv.capacity(), fb.capacity());
        BOOST_TEST_EQ(dbv.capacity(), 64u);
        BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
        BOOST_TEST_EQ(dbv.capacity(), fb.capacity());
        BOOST_TEST_EQ(dbv.capacity(), 64u);
        dbv.commit(64);
        BOOST_TEST_EQ(dbv.capacity(), fb.size());
        BOOST_TEST_EQ(dbv.size(), 64u);
        BOOST_TEST_EQ(fb.data().size(), 64u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

        dbv.consume(32);
        BOOST_TEST_EQ(dbv.capacity(), fb.size());
        BOOST_TEST_EQ(dbv.size(), 32u);
        BOOST_TEST_EQ(buffer_size(fb.data()), 32u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 32u);
        BOOST_TEST(
            std::equal(asio::buffers_begin(dbv.data()),
                       asio::buffers_end(dbv.data()),
                       data.begin() + 32,
                       data.end()));

      BOOST_TEST(std::memcmp(fb.data().data(), data.data(), fb.data().size()));
    }


    void testString()
    {
      std::vector<char> vec;
      dynamic_buffer_view dbv{vec};

      std::string data;
      data.resize(64);
      char c = ' ';
      std::generate(data.begin(), data.end(), [&]{return c++;});

      BOOST_TEST_EQ(dbv.capacity(), vec.size());
      BOOST_TEST_EQ(dbv.capacity(), 0u);
      BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
      BOOST_TEST_EQ(dbv.capacity(), vec.size());
      BOOST_TEST_GE(dbv.capacity(), 64u);
      dbv.commit(64);
      BOOST_TEST_EQ(dbv.capacity(), vec.size());
      BOOST_TEST_GE(dbv.size(), 64u);
      BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

      BOOST_TEST_EQ(dbv.size(), 64u);
      dbv.consume(32);
      BOOST_TEST_GE(dbv.capacity(), vec.size());
      BOOST_TEST_EQ(dbv.size(), 32u);
      BOOST_TEST_EQ(buffer_size(dbv.data()), 32u);

      BOOST_TEST(
          std::equal(asio::buffers_begin(dbv.data()),
                     asio::buffers_end(dbv.data()),
                     data.begin() + 32,
                     data.end()));

      BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
      BOOST_TEST_EQ(dbv.capacity(), vec.capacity());
      BOOST_TEST_GE(dbv.capacity(), 64u);
      dbv.commit(64);
      BOOST_TEST_EQ(dbv.capacity(), vec.capacity());
      BOOST_TEST_GE(dbv.size(), 64u);
      BOOST_TEST_EQ(buffer_size(dbv.data()), 96u);

      BOOST_TEST_EQ(dbv.size(), 96u);
      dbv.consume(32);
      BOOST_TEST_GE(dbv.capacity(), vec.size());
      BOOST_TEST_EQ(dbv.size(), 64);
      BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

      BOOST_TEST(
          std::equal(asio::buffers_begin(dbv.data()),
                     asio::buffers_end(dbv.data()),
                     data.begin(),
                     data.end()));
    }

    void testCircular()
    {
        boost::circular_buffer<char> vec{64u};
        dynamic_buffer_view dbv{vec};

        std::string data;
        data.resize(64);
        char c = ' ';
        std::generate(data.begin(), data.end(), [&]{return c++;});

        BOOST_TEST_EQ(buffer_size(dbv.prepare(64)), 64u);
        BOOST_TEST_EQ(dbv.capacity(), vec.capacity());
        BOOST_TEST_EQ(dbv.capacity(), 64u);
        BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.capacity(), 64u);
        dbv.commit(64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.size(), 64u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

        BOOST_TEST_EQ(dbv.size(), 64u);
        dbv.consume(32);
        BOOST_TEST_GE(dbv.capacity(), vec.size());
        BOOST_TEST_EQ(dbv.size(), 32u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 32u);

        BOOST_TEST(
            std::equal(asio::buffers_begin(dbv.data()),
                       asio::buffers_end(dbv.data()),
                       data.begin() + 32,
                       data.end()));

        BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.capacity(), 64u);
        dbv.commit(64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.size(), 64u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 96u);

        BOOST_TEST_EQ(dbv.size(), 96u);
        dbv.consume(32);
        BOOST_TEST_GE(dbv.capacity(), vec.size());
        BOOST_TEST_EQ(dbv.size(), 64);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

        BOOST_TEST(
            std::equal(asio::buffers_begin(dbv.data()),
                       asio::buffers_end(dbv.data()),
                       data.begin(),
                       data.end()));
    }


    void testQueue()
    {
        boost::container::deque<char, void,
            boost::container::deque_options_t<boost::container::block_bytes<6u>>> vec;
        dynamic_buffer_view dbv{vec};

        std::string data;
        data.resize(64);
        char c = ' ';
        std::generate(data.begin(), data.end(), [&]{return c++;});

        BOOST_TEST_EQ(buffer_size(dbv.prepare(64)), 64u);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_EQ(dbv.capacity(), 64u);
        BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.capacity(), 64u);
        dbv.commit(64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.size(), 64u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

        BOOST_TEST_EQ(dbv.size(), 64u);
        dbv.consume(32);
        BOOST_TEST_GE(dbv.capacity(), vec.size());
        BOOST_TEST_EQ(dbv.size(), 32u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 32u);

        BOOST_TEST(
            std::equal(asio::buffers_begin(dbv.data()),
                       asio::buffers_end(dbv.data()),
                       data.begin() + 32,
                       data.end()));

        BOOST_TEST_EQ(asio::buffer_copy(dbv.prepare(64), asio::buffer(data)), 64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.capacity(), 64u);
        dbv.commit(64);
        BOOST_TEST_EQ(dbv.capacity(), vec.size());
        BOOST_TEST_GE(dbv.size(), 64u);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 96u);

        BOOST_TEST_EQ(dbv.size(), 96u);
        dbv.consume(32);
        BOOST_TEST_GE(dbv.capacity(), vec.size());
        BOOST_TEST_EQ(dbv.size(), 64);
        BOOST_TEST_EQ(buffer_size(dbv.data()), 64u);

        BOOST_TEST(
            std::equal(asio::buffers_begin(dbv.data()),
                       asio::buffers_end(dbv.data()),
                       data.begin(),
                       data.end()));
    }

    void
    run()
    {
        testOther();
        testString();
        testCircular();
        testQueue();
    }
};

TEST_SUITE(
    dynamic_buffer_view_test,
    "boost.buffers.dynamic_buffer_view");

} // boost::buffers
