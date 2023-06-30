//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_TEST_HELPERS_HPP
#define BOOST_BUFFERS_TEST_HELPERS_HPP

#include <boost/async/io/buffers/algorithm.hpp>
#include <boost/async/io/buffers/buffer.hpp>
#include <boost/async/io/buffers/buffer_copy.hpp>
#include <boost/async/io/buffers/buffer_size.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <string>
#include "test_suite.hpp"

namespace boost::async::io::buffers {

inline
std::string const&
test_pattern()
{
    static std::string const pat =
        "012" "34567" "89abcde";
    return pat;
}

template<class Buffers>
std::string
test_to_string(Buffers const& bs)
{
    std::string s(
        buffer_size(bs), 0);
    s.resize(buffer_copy(
        buffer(&s[0], s.size()),
        bs));
    return s;
}

template<class T>
void
test_buffer_sequence(T&& t)
{
    static_assert(const_buffer_sequence<std::decay_t<T>>);

    auto const& pat = test_pattern();
    auto const& ct = t;

    // operator++()
    {
        std::string s;
        auto it = begin(t);
        auto const end_ = end(t);
        while(it != end_)
        {
            auto b = *it;
            s.append(static_cast<
                char const*>(b.data()),
                b.size());
            ++it;
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator++(int)
    {
        std::string s;
        auto it = begin(t);
        auto const end_ = end(t);
        while(it != end_)
        {
            auto b = *it;
            s.append(static_cast<
                char const*>(b.data()),
                b.size());
            it++;
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator++() const
    {
        std::string s;
        auto it = begin(ct);
        auto const end_ = end(ct);
        while(it != end_)
        {
            auto b = *it;
            s.append(static_cast<
                char const*>(b.data()),
                b.size());
            ++it;
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator++(int) const
    {
        std::string s;
        auto it = begin(ct);
        auto const end_ = end(ct);
        while(it != end_)
        {
            auto b = *it;
            s.append(static_cast<
                char const*>(b.data()),
                b.size());
            it++;
        }
        BOOST_TEST_EQ(s, pat);
    }

    // operator--()
    {
        std::string s;
        auto it = end(t);
        auto const begin_ = begin(t);
        while(it != begin_)
        {
            --it;
            auto b = *it;
            s.insert(0, static_cast<
                char const*>(b.data()),
                b.size());
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator--(int)
    {
        std::string s;
        auto it = end(t);
        auto const begin_ = begin(t);
        while(it != begin_)
        {
            it--;
            auto b = *it;
            s.insert(0, static_cast<
                char const*>(b.data()),
                b.size());
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator--() const
    {
        std::string s;
        auto it = end(ct);
        auto const begin_ = begin(ct);
        while(it != begin_)
        {
            --it;
            auto b = *it;
            s.insert(0, static_cast<
                char const*>(b.data()),
                b.size());
        }
        BOOST_TEST_EQ(s, pat);
    }
    // operator--(int) const
    {
        std::string s;
        auto it = end(ct);
        auto const begin_ = begin(ct);
        while(it != begin_)
        {
            it--;
            auto b = *it;
            s.insert(0, static_cast<
                char const*>(b.data()),
                b.size());
        }
        BOOST_TEST_EQ(s, pat);
    }

    // prefix
    for(std::size_t i = 0;
        i <= pat.size(); ++i)
        BOOST_TEST_EQ(
            test_to_string(prefix(t, i)),
            pat.substr(0, i));
    BOOST_TEST_EQ(test_to_string(prefix(
        t, std::size_t(-1))), pat);

    // prefix
    for(std::size_t i = 0;
        i <= pat.size(); ++i)
        BOOST_TEST_EQ(
            test_to_string(prefix(ct, i)),
            pat.substr(0, i));
    BOOST_TEST_EQ(test_to_string(prefix(
        ct, std::size_t(-1))), pat);

    // suffix
    for(std::size_t i = 0;
        i <= pat.size(); ++i)
        BOOST_TEST_EQ(
            test_to_string(suffix(t, i)),
            pat.substr(pat.size() - i, i));
    BOOST_TEST_EQ(test_to_string(suffix(
        t, std::size_t(-1))), pat);

    // suffix
    for(std::size_t i = 0;
        i <= pat.size(); ++i)
        BOOST_TEST_EQ(
            test_to_string(suffix(ct, i)),
            pat.substr(pat.size() - i, i));
    BOOST_TEST_EQ(test_to_string(suffix(
        ct, std::size_t(-1))), pat);
}

} // boost::buffers

#endif
