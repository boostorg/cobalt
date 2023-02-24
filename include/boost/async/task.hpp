//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_TASK_HPP
#define BOOST_ASYNC_TASK_HPP

#include <boost/async/detail/task.hpp>

#include <boost/asio/append.hpp>

namespace boost::async
{

template<typename Return>
struct [[nodiscard]] task
{
    using promise_type = detail::task_promise<Return>;

    task(const task &) = delete;
    task& operator=(const task &) = delete;

    task(task &&lhs) noexcept = default;
    task& operator=(task &&) noexcept = default;

    auto operator co_await () && {return receiver_.get_awaitable();}

    ~task() {}
  private:
    template<typename>
    friend struct detail::task_promise;

    task(detail::task_promise<Return> * task) : receiver_(task)
    {
    }

    detail::task_receiver<Return> receiver_;
    friend struct detail::async_initiate;
};



}


#endif //BOOST_ASYNC_ASYNC_HPP
