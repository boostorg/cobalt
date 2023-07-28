//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_MUTABLE_BUFFER_HPP
#define BOOST_ASYNC_IO_BUFFERS_MUTABLE_BUFFER_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/tag_invoke.hpp>
#include <cstddef>
#include <type_traits>

namespace boost::async::io::buffers {

/** Holds a buffer that can be modified.
*/
class mutable_buffer
{
    unsigned char* p_ = nullptr;
    std::size_t n_ = 0;

public:
    using value_type = mutable_buffer;
    using const_iterator =
        value_type const*;

    /** Constructor.
    */
    mutable_buffer() = default;

    /** Constructor.
    */
    mutable_buffer(
        mutable_buffer const&) = default;

    /** Constructor.
    */
    mutable_buffer(
        void* data,
        std::size_t size) noexcept
        : p_(static_cast<
            unsigned char*>(data))
        , n_(size)
    {
    }

    /** Constructor
     */
    template<typename Container>
    requires (
        requires (Container & ct)
        {
          {std::data(ct)} -> std::convertible_to<void*>;
          {std::size(ct)} -> std::convertible_to<std::size_t>;
        }
        && std::is_trivial_v<typename Container::value_type>)
      mutable_buffer(Container & ct) : mutable_buffer(std::data(ct), sizeof(typename Container::value_type) * std::size(ct)) {}

    /** Constructor for arrays */
    template<typename T, std::size_t N>
      requires std::is_trivial_v<T>
    mutable_buffer(T (&arr)[N]) : mutable_buffer(&arr[0], sizeof(T) * N) {}

    /** Constructor for arrays */
    template<typename T>
      requires std::is_trivial_v<T>
    mutable_buffer(std::pair<T*, std::size_t> p) : mutable_buffer(p.first, sizeof(T) * p.second) {}


    /** Assignment.
    */
    mutable_buffer& operator=(
        mutable_buffer const&) = default;

#ifndef BOOST_ASYNC_IO_BUFFERS_DOCS
    // conversion to boost::asio::mutable_buffer
    template<class T>
    requires (std::constructible_from<T, void*, std::size_t>
           && !std::same_as<T, mutable_buffer>)
    operator T() const noexcept
    {
        return T{ data(), size() };
    }
#endif

    void*
    data() const noexcept
    {
        return p_;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    const_iterator
    begin() const noexcept
    {
        return this;
    }

    const_iterator
    end() const noexcept
    {
        return this + 1;
    }

    /** Remove a prefix from the buffer.
    */
    mutable_buffer&
    operator+=(std::size_t n) noexcept
    {
        if(n >= n_)
        {
            p_ = p_ + n_;
            n_ = 0;
            return *this;
        }
        p_ = p_ + n;
        n_ -= n;
        return *this;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    mutable_buffer
    operator+(
        mutable_buffer b,
        std::size_t n) noexcept
    {
        return b += n;
    }

    /** Return the buffer with a prefix removed.
    */
    friend
    mutable_buffer
    operator+(
        std::size_t n,
        mutable_buffer b) noexcept
    {
        return b += n;
    }

#ifndef BOOST_BUFFER_DOCS
    friend
    mutable_buffer
    tag_invoke(
        prefix_tag const&,
        mutable_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_, n };
        return b;
    }

    friend
    mutable_buffer
    tag_invoke(
        suffix_tag const&,
        mutable_buffer const& b,
        std::size_t n) noexcept
    {
        if(n < b.size())
            return { b.p_ + b.n_ - n, n };
        return b;
    }
#endif
};

} // boost::buffers

#endif
