//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

// Test that header file is self-contained.
#include <boost/async/io/buffers/algorithm.hpp>

#include <boost/async/io/buffers/buffer_copy.hpp>
#include <boost/async/io/buffers/buffer_size.hpp>
#include <boost/async/io/buffers/const_buffer_pair.hpp>
#include <boost/async/io/buffers/mutable_buffer_pair.hpp>
#include <boost/static_assert.hpp>
#include <string>
#include "test_suite.hpp"

namespace boost::async::io::buffers {

struct asio_mutable_buffer
{
    std::size_t size() const noexcept { return 0; }
    void* data() const noexcept { return nullptr; }
};

struct asio_const_buffer
{
    std::size_t size() const noexcept { return 0; }
    void const* data() const noexcept { return nullptr; }
};

struct not_a_buffer
{
    std::size_t size() const noexcept;
    char* data() const noexcept;
};

struct asio_mutable_buffers
{
    asio_mutable_buffer const* begin() const noexcept;
    asio_mutable_buffer const* end() const noexcept;
};

struct asio_const_buffers
{
    asio_const_buffer const* begin() const noexcept;
    asio_const_buffer const* end() const noexcept;
};

BOOST_STATIC_ASSERT(  const_buffer_sequence   <const_buffer>);
BOOST_STATIC_ASSERT(  const_buffer_sequence   <mutable_buffer>);
BOOST_STATIC_ASSERT(! mutable_buffer_sequence <const_buffer>);
BOOST_STATIC_ASSERT(  mutable_buffer_sequence <mutable_buffer>);

//BOOST_STATIC_ASSERT(  is_const_buffer_sequence   <asio_const_buffers>::value);
//BOOST_STATIC_ASSERT(  is_const_buffer_sequence   <asio_mutable_buffers>::value);
//BOOST_STATIC_ASSERT(  is_mutable_buffer_sequence <asio_mutable_buffers>::value);
//BOOST_STATIC_ASSERT(! is_mutable_buffer_sequence <asio_const_buffers>::value);

BOOST_STATIC_ASSERT(  const_buffer_sequence   <const_buffer_pair>);
BOOST_STATIC_ASSERT(  const_buffer_sequence   <mutable_buffer_pair>);
BOOST_STATIC_ASSERT(! mutable_buffer_sequence <const_buffer_pair>);
BOOST_STATIC_ASSERT(  mutable_buffer_sequence <mutable_buffer_pair>);

struct algorithm_test
{
    void
    testBufferSize()
    {
        {
            char a[7]{};
            char b[11]{};
            const_buffer_pair p(
                const_buffer(a, sizeof(a)),
                const_buffer(b, sizeof(b)));
            BOOST_TEST_EQ(
                buffer_size(p),
                sizeof(a) + sizeof(b));
        }
    }

    void
    testBufferCopy()
    {
        std::string const s =
            "Howdy partner";
        auto const N = s.size();
        for(std::size_t i = 0;
            i < N; ++i)
        {
            for(std::size_t j = 0;
                j < N; ++j)
            {
                for(std::size_t k = 0;
                    k < N + 2; ++k)
                {
                    const_buffer_pair p0(
                        const_buffer(
                            s.data(), i),
                        const_buffer(
                            s.data() + i, N - i));
                    char tmp[13];
                    std::memset(tmp, 0, sizeof(tmp));
                    mutable_buffer_pair p1(
                        mutable_buffer(
                            tmp, j),
                        mutable_buffer(
                            tmp + j, N - j));
                    auto const n = buffer_copy(
                        p1, p0, k);
                    BOOST_TEST_LE(n, N);
                    BOOST_TEST_EQ(
                        s.substr(0, n),
                        std::string(tmp, n));
                }
            }
        }
    }

    void
    testAlgorithms()
    {
        // prefix
        
        {
            char buf[16]{};
            const_buffer b(buf, sizeof(buf));
            const_buffer bp = prefix(b, 5);
            BOOST_TEST_EQ(bp.size(), 5);
        }
 
        {
            char buf[16]{};
            mutable_buffer b(buf, sizeof(buf));
            mutable_buffer bp = prefix(b, 5);
            BOOST_TEST_EQ(bp.size(), 5);
        }

        // sans_prefix
        
        {
            char buf[16]{};
            const_buffer b(buf, sizeof(buf));
            const_buffer bp = sans_prefix(b, 5);
            BOOST_TEST_EQ(bp.size(), 11);
        }
 
        {
            char buf[16]{};
            mutable_buffer b(buf, sizeof(buf));
            mutable_buffer bp = sans_prefix(b, 5);
            BOOST_TEST_EQ(bp.size(), 11);
        }

        // suffix

        {
            char buf[16]{};
            const_buffer b(buf, sizeof(buf));
            const_buffer bp = suffix(b, 5);
            BOOST_TEST_EQ(bp.size(), 5);
        }

        {
            char buf[16]{};
            mutable_buffer b(buf, sizeof(buf));
            mutable_buffer bp = suffix(b, 5);
            BOOST_TEST_EQ(bp.size(), 5);
        }

        // sans_suffix

        {
            char buf[16]{};
            const_buffer b(buf, sizeof(buf));
            const_buffer bp = sans_suffix(b, 5);
            BOOST_TEST_EQ(bp.size(), 11);
        }

        {
            char buf[16]{};
            mutable_buffer b(buf, sizeof(buf));
            mutable_buffer bp = sans_suffix(b, 5);
            BOOST_TEST_EQ(bp.size(), 11);
        }
    }

    void
    run()
    {
        testBufferSize();
        testBufferCopy();
        testAlgorithms();
    }
};

TEST_SUITE(
    algorithm_test,
    "boost.buffers.algorithm");

} // boost::buffers
