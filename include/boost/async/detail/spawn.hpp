//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_DETAIL_SPAWN_HPP
#define BOOST_ASYNC_DETAIL_SPAWN_HPP

#include <boost/async/task.hpp>

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
  void operator()(Handler && h, task<T> a, executor_type exec)
  {
    auto & rec = a.receiver_;
    if (rec.done)
      return asio::post(asio::append(h, rec.exception, rec.exception ? T() : rec.get_result()));

    auto dalloc = container::pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()};
    auto alloc = asio::get_associated_allocator(h, dalloc);
    auto recs = allocate_unique<detail::task_receiver<T>>(alloc, std::move(rec));

    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
      sl.template emplace<detail::forward_cancellation>(recs->promise->signal);

    auto p = recs.get();

    p->promise->exec.emplace(exec);
    p->awaited_from.reset(detail::post_coroutine(
        asio::bind_executor(
            asio::get_associated_executor(h, exec),
            asio::bind_allocator(
                alloc,
                [r = std::move(recs),
                 h = std::move(h)]() mutable
                {
                  auto ex = r->exception;
                  auto rr = std::move(r->get_result());
                  r.reset();
                  h(ex, std::move(rr));
                }
            )
        )
    ).address());

    asio::post(exec, std::coroutine_handle<detail::task_promise<T>>::from_promise(*p->promise));
  }

  template<typename Handler>
  void operator()(Handler && h, task<void> a, executor_type exec)
  {
    if (a.receiver_.done)
      return asio::post(asio::append(h, a.receiver_.exception));

    auto alloc = asio::get_associated_allocator(h, container::pmr::polymorphic_allocator<void>{boost::async::this_thread::get_default_resource()});
    auto recs = allocate_unique<detail::task_receiver<void>>(alloc, std::move(a.receiver_));

    if (recs->done)
      return asio::post(asio::append(h, recs->exception));

    auto sl = asio::get_associated_cancellation_slot(h);
    if (sl.is_connected())
      sl.template emplace<detail::forward_cancellation>(recs->promise->signal);

    auto p = recs.get();

    p->promise->exec.emplace(exec);
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
    
    asio::post(exec, std::coroutine_handle<detail::task_promise<void>>::from_promise(*p->promise));
  }


  template<typename Handler, typename T>
  struct task_type
  {
    auto initial_suspend() noexcept { return std::suspend_never(); }
    auto final_suspend() noexcept { return std::suspend_never(); }
  };
};

}

#endif //BOOST_ASYNC_DETAIL_SPAWN_HPP
