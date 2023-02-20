// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_WAIT_GROUP_HPP
#define BOOST_ASYNC_WAIT_GROUP_HPP

#include <boost/async/detail/wait_group.hpp>

namespace boost::async
{

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

    detail::wait_wrapper wait()
    {
        return detail::wait_wrapper(waitables_);

    }

    detail::wait_wrapper operator co_await ()
    {
      return detail::wait_wrapper(waitables_);
    }
    /// swallow the exception here.
    detail::wait_wrapper await_exit(std::exception_ptr ep)
    {
        auto ct = ep ? ct_except_ : ct_normal_;
        if (ct != asio::cancellation_type::none || ep)
            for (auto & w : waitables_)
                w.cancel(ct);
        return detail::wait_wrapper(waitables_);
    }

    std::size_t size() const {return waitables_.size();}
    // remove completed tasks
    std::size_t reap()
    {
        return erase_if(waitables_, [](promise<void> & p) { return p.ready() && p;});
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
