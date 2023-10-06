//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/buffers
//

#ifndef BOOST_ASYNC_IO_BUFFERS_RANGE_HPP
#define BOOST_ASYNC_IO_BUFFERS_RANGE_HPP

#include <boost/async/config.hpp>
#include <boost/async/io/buffers/const_buffer.hpp>
#include <boost/async/io/buffers/mutable_buffer.hpp>
#include <boost/async/io/buffers/concepts.hpp>
#include <type_traits>

namespace boost::async::io::buffers {

#ifdef BOOST_ASYNC_IO_BUFFERS_DOCS

/** Return an iterator to the beginning of the buffer sequence.
*/
template<class BufferSequence>
__see_below__
begin(BufferSequence&& b) noexcept;

/** Return an iterator to the end of the buffer sequence.
*/
template<class BufferSequence>
__see_below__
end(BufferSequence&& b) noexcept;

/** Return a range representing the buffer sequence.
*/
template<class BufferSequence>
__see_below__
range(BufferSequence&& bs) noexcept;

#else

namespace detail {

struct begin_impl
{
    template<class MutableBuffer>
    auto
    operator()(
        MutableBuffer const& b) const noexcept ->
            typename std::enable_if<
            std::is_convertible<
                MutableBuffer const*,
                mutable_buffer const*>::value,
            mutable_buffer const*>::type
    {
        return static_cast<
            mutable_buffer const*>(
                std::addressof(b));
    }

    template<class ConstBuffer>
    auto
    operator()(
        ConstBuffer const& b) const noexcept ->
            typename std::enable_if<
            std::is_convertible<
                ConstBuffer const*,
                const_buffer const*>::value,
            const_buffer const*>::type
    {
        return static_cast<
            const_buffer const*>(
                std::addressof(b));
    }

    template<class BufferSequence>
    auto
    operator()(
        BufferSequence& bs) const noexcept ->
            typename std::enable_if<
            ! std::is_convertible<
                BufferSequence const*,
                const_buffer const*>::value &&
            ! std::is_convertible<
                BufferSequence const*,
                mutable_buffer const*>::value,
            decltype(bs.begin())>::type
    {
        return bs.begin();
    }

    template<class BufferSequence>
    auto
    operator()(
        BufferSequence const& bs) const noexcept ->
            typename std::enable_if<
            ! std::is_convertible<
                BufferSequence const*,
                const_buffer const*>::value &&
            ! std::is_convertible<
                BufferSequence const*,
                mutable_buffer const*>::value,
            decltype(bs.begin())>::type
    {
        return bs.begin();
    }
};

struct end_impl
{
    template<class MutableBuffer>
    auto
    operator()(
        MutableBuffer const& b) const noexcept ->
            typename std::enable_if<
            std::is_convertible<
                MutableBuffer const*,
                mutable_buffer const*>::value,
            mutable_buffer const*>::type
    {
        return static_cast<
            mutable_buffer const*>(
                std::addressof(b)) + 1;
    }

    template<class ConstBuffer>
    auto
    operator()(
        ConstBuffer const& b) const noexcept ->
            typename std::enable_if<
            std::is_convertible<
                ConstBuffer const*,
                const_buffer const*>::value,
            const_buffer const*>::type
    {
        return static_cast<
            const_buffer const*>(
                std::addressof(b)) + 1;
    }

    template<class BufferSequence>
    auto
    operator()(
        BufferSequence& bs) const noexcept ->
            typename std::enable_if<
            ! std::is_convertible<
                BufferSequence const*,
                const_buffer const*>::value &&
            ! std::is_convertible<
                BufferSequence const*,
                mutable_buffer const*>::value,
            decltype(bs.end())>::type
    {
        return bs.end();
    }

    template<class BufferSequence>
    auto
    operator()(
        BufferSequence const& bs) const noexcept ->
            typename std::enable_if<
            ! std::is_convertible<
                BufferSequence const*,
                const_buffer const*>::value &&
            ! std::is_convertible<
                BufferSequence const*,
                mutable_buffer const*>::value,
            decltype(bs.end())>::type
    {
        return bs.end();
    }
};

} // detail

constexpr detail::begin_impl begin{};
constexpr detail::end_impl end{};

//------------------------------------------------

namespace detail {

template<class T>
class iter_range
{
    using begin_type = decltype(
        buffers::begin(std::declval<T&>()));
    using end_type = decltype(
        buffers::end(std::declval<T&>()));

    begin_type begin_;
    end_type end_;

public:
    iter_range(T& t) noexcept
        : begin_(buffers::begin(t))
        , end_(buffers::end(t))
    {
    }

    begin_type
    begin() const noexcept
    {
        return begin_;
    }

    end_type
    end() const noexcept
    {
        return end_;
    }
};

struct range_impl
{
    template<class BufferSequence>
    auto
    operator()(
        BufferSequence&& bs) const noexcept ->
            iter_range<typename
                std::remove_reference<
                    BufferSequence>::type>
    {
        return { bs };
    }
};

} // detail

constexpr detail::range_impl range{};

#endif

} // boost::buffers

#endif
