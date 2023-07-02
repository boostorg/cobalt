//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_BUFFERS_ANY_DYNAMIC_BUFFER_HPP
#define BOOST_BUFFERS_ANY_DYNAMIC_BUFFER_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer_span.hpp>
#include <boost/async/io/buffers/mutable_buffer_span.hpp>
#include <boost/async/io/buffers/range.hpp>
#include <boost/async/io/buffers/type_traits.hpp>
#include <cstdlib>

namespace boost::async::io::buffers {

/** An abstract, type-erased dynamic buffer.
*/
struct any_dynamic_buffer
{
    using const_buffers_type =
        buffers::const_buffer_span;

    using mutable_buffers_type =
        buffers::mutable_buffer_span;

    virtual ~any_dynamic_buffer() = default;
    virtual std::size_t size() const = 0;
    virtual std::size_t max_size() const = 0;
    virtual std::size_t capacity() const = 0;
    virtual const_buffers_type data() const = 0;
    virtual mutable_buffers_type prepare(std::size_t) = 0;
    virtual void commit(std::size_t) = 0;
    virtual void consume(std::size_t) = 0;
};

//-----------------------------------------------

/** A type-erased dynamic buffer.
*/
template<
    class DynamicBuffer,
    std::size_t N = asio::detail::max_iov_len>
class any_dynamic_buffer_impl
    : public any_dynamic_buffer
{
    DynamicBuffer b_;
    buffers::const_buffer data_[N];
    buffers::mutable_buffer out_[N];
    std::size_t data_len_ = 0;
    std::size_t out_len_ = 0;

    template<class Buffers>
    static
    std::size_t
    unroll(
        Buffers const& bs,
        value_type<Buffers>* dest,
        std::size_t len)
    {
        std::size_t i = 0;
        for(auto b : buffers::range(bs))
        {
            dest[i++] = b;
            if(i == len)
                break;
        }
        return i;
    }

public:
    template<class DynamicBuffer_>
    explicit
    any_dynamic_buffer_impl(
        DynamicBuffer_&& b)
        : b_(std::forward<
            DynamicBuffer_>(b))
    {
    }

    DynamicBuffer&
    buffer() noexcept
    {
        return b_;
    }

    DynamicBuffer const&
    buffer() const noexcept
    {
        return b_;
    }

    std::size_t
    size() const override
    {
        return b_.size();
    }

    std::size_t
    max_size() const override
    {
        return b_.max_size();
    }

    std::size_t
    capacity() const override
    {
        return b_.capacity();
    }

    const_buffers_type
    data() const override
    {
        return const_buffers_type(
            data_, data_len_);
    }

    auto
    prepare(
        std::size_t n) ->
            mutable_buffers_type override
    {
        out_len_ = unroll(
            b_.prepare(n), out_, N);
        return mutable_buffers_type(
            out_, out_len_);
    }

    void
    commit(
        std::size_t n) override
    {
        b_.commit(n);
        data_len_ = unroll(
            b_.data(), data_, N);
    }

    void
    consume(
        std::size_t n) override
    {
        b_.consume(n);
        data_len_ = unroll(
            b_.data(), data_, N);
    }
};

template<
    class DynamicBuffer
#ifndef BOOST_BUFFERS_DOCS
    , class = typename std::enable_if<
        is_dynamic_buffer<
        typename std::decay<DynamicBuffer>::type
            >::value>::type
#endif
>
auto
make_any(DynamicBuffer&& b) ->
    any_dynamic_buffer_impl<typename
        std::decay<DynamicBuffer>::type>
{
    return any_dynamic_buffer_impl<typename
        std::decay<DynamicBuffer>::type>(
            std::forward<DynamicBuffer>(b));
}

} // boost::buffers

#endif
