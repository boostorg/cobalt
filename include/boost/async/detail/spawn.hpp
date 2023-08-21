//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_SPAWN_HPP
#define BOOST_ASYNC_DETAIL_SPAWN_HPP

#include <boost/async/task.hpp>
#include <boost/asio/dispatch.hpp>

#include <boost/smart_ptr/allocate_unique.hpp>

namespace boost::async
{
template<typename T>
struct task;
}

namespace boost::async::detail
{

struct async_initiate
{
  template<typename Handler, typename T>
  void operator()(Handler && h, task<T> a, executor exec)
  {
    auto & rec = a.receiver_;
    if (rec.done)
      return asio::dispatch(
          asio::get_associated_immediate_executor(h, exec),
          asio::append(std::forward<Handler>(h), rec.exception, rec.exception ? T() : rec.get_result()));

#if !defined(BOOST_ASYNC_NO_PMR)
    auto dalloc = pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()};
    auto alloc = asio::get_associated_allocator(h, dalloc);
#else
    auto alloc = asio::get_associated_allocator(h);
#endif
    auto recs = allocate_unique<detail::task_receiver<T>>(alloc, std::move(rec));

    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
      sl.template emplace<detail::forward_dispatch_cancellation>(recs->promise->signal, exec);

    auto p = recs.get();

    p->promise->exec.emplace(exec);
    p->promise->exec_ = exec;
    p->awaited_from.reset(detail::post_coroutine(
        asio::bind_executor(
            asio::get_associated_executor(h, exec),
            asio::bind_allocator(
                alloc,
                [r = std::move(recs),
                 h = std::move(h)]() mutable
                {

                  auto ex = r->exception;
                  T rr{};
                  if (r->result)
                     rr = r->get_result();
                  r.reset();
                  h(ex, std::move(rr));
                }
            )
        )
    ).address());

    asio::dispatch(exec, std::coroutine_handle<detail::task_promise<T>>::from_promise(*p->promise));
  }

  template<typename Handler>
  void operator()(Handler && h, task<void> a, executor exec)
  {
    if (a.receiver_.done)
      return asio::dispatch(
          asio::get_associated_immediate_executor(h, exec),
          asio::append(std::forward<Handler>(h), a.receiver_.exception));


#if !defined(BOOST_ASYNC_NO_PMR)
    auto alloc = asio::get_associated_allocator(h, pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()});
#else
    auto alloc = asio::get_associated_allocator(h);
#endif
    auto recs = allocate_unique<detail::task_receiver<void>>(alloc, std::move(a.receiver_));

    if (recs->done)
      return asio::dispatch(asio::get_associated_immediate_executor(h, exec),
                            asio::append(std::forward<Handler>(h), recs->exception));

    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
      sl.template emplace<detail::forward_dispatch_cancellation>(recs->promise->signal, exec);

    auto p = recs.get();

    p->promise->exec.emplace(exec);
    p->promise->exec_ = exec;
    p->awaited_from.reset(detail::post_coroutine(
        asio::bind_executor(
            asio::get_associated_executor(h, exec),
            asio::bind_allocator(
                alloc,
                [r = std::move(recs),
                 h = std::move(h)]() mutable
                {
                  auto ex = r->exception;
                  r.reset();
                  h(ex);
                }
            )
        )
    ).address());
    
    asio::dispatch(exec, std::coroutine_handle<detail::task_promise<void>>::from_promise(*p->promise));
  }
};

}

#endif //BOOST_ASYNC_DETAIL_SPAWN_HPP
