// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_WAIT_GROUP_HPP
#define BOOST_ASYNC_WAIT_GROUP_HPP

#include <boost/async/promise.hpp>
#include <list>

namespace boost::async
{

namespace detail
{

struct select_wrapper
{
    using impl_type = decltype(select(std::declval<std::list<promise<void>> &>()));
    std::list<promise<void>> &waitables_;
    std::optional<impl_type> impl_;

    select_wrapper(std::list<promise<void>> &waitables) : waitables_(waitables)
    {
        if (!waitables_.empty())
            impl_.emplace(select(waitables_));

    }

    bool await_ready()
    {
        if (waitables_.empty())
            return true;
        else
            return impl_->await_ready();
    }

    template<typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> h)
    {
        return impl_->await_suspend(h);
    }

    void await_resume()
    {
        if (waitables_.empty())
            return;
        auto idx = impl_->await_resume();
        if (idx != -1)
            waitables_.erase(std::next(waitables_.begin(), idx));
    }
};

}

struct wait_group
{
    wait_group(asio::cancellation_type normal_cancel = asio::cancellation_type::none,
               asio::cancellation_type exception_cancel = asio::cancellation_type::all)
               : ct_normal_(normal_cancel), ct_except_(exception_cancel) {}

    void push_back(promise<void> p) { waitables_.push_back(std::move(p));}

    auto wait_one() -> detail::select_wrapper
    {
        return  detail::select_wrapper(waitables_);
    }

    promise<void> wait()
    {
        co_await async::wait(waitables_);
    }

    auto operator co_await () { return async::wait(waitables_);}
    /// swallow the exception here.
    auto await_exit(std::exception_ptr ep)
    {
        auto ct = ep ? ct_except_ : ct_normal_;
        if (ct != asio::cancellation_type::none || ep)
            for (auto & w : waitables_)
                w.cancel(ct);
        return async::wait(waitables_);
    }

    std::size_t size() const {return waitables_.size();}
    // remove completed tasks
    std::size_t reap()
    {
        return erase_if(waitables_, [](promise<void> & p) { return p.ready();});
    }
    void cancel(asio::cancellation_type ct = asio::cancellation_type::all)
    {
        for (auto & w : waitables_)
            w.cancel(ct);
    }

  private:
    std::list<promise<void>> waitables_;
    asio::cancellation_type ct_normal_, ct_except_;
};

}

#endif //BOOST_ASYNC_WAIT_GROUP_HPP
