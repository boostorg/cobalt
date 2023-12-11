//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_COMPOSED_HPP
#define BOOST_COBALT_COMPOSED_HPP

#include <boost/cobalt/detail/handler.hpp>

namespace boost::cobalt::detail
{

template<typename ... Args>
struct composed_promise_return
{
  completion_handler<Args...> & handler;

  void return_value(std::tuple<Args...> && tup)
  {
    handler.result.emplace(std::move(tup));
  }
};


template<>
struct composed_promise_return<>
{
  completion_handler<> & handler;

  void return_void()
  {
    handler.result.emplace();
  }
};

template<typename ... Args>
struct composed_promise : composed_promise_return<Args...>
{
#if !defined(BOOST_COBALT_NO_PMR)
  template<typename ... Args_>
  void * operator new(const std::size_t size, Args_ & ... args)
  {
    auto res = std::get<sizeof...(Args_) - 1>(std::tie(args...)).get_allocator().resource();
    const auto p = res->allocate(size + sizeof(pmr::memory_resource *), alignof(pmr::memory_resource *));
    auto pp = static_cast<pmr::memory_resource**>(p);
    *pp = res;
    return pp + 1;
  }

  void operator delete(void * raw, const std::size_t size) noexcept
  {
    const auto p = static_cast<pmr::memory_resource**>(raw) - 1;
    pmr::memory_resource * res = *p;
    res->deallocate(p, size + sizeof(pmr::memory_resource *), alignof(pmr::memory_resource *));
  }
#endif

  template<typename ... Args_>
  composed_promise(Args_ & ... args)
        : composed_promise_return<Args...>(std::get<sizeof...(Args_) - 1u>(std::tie(args...))) {}

  using executor_type = executor;
  const executor_type & get_executor() const {return this->handler.get_executor();}

#if !defined(BOOST_COBALT_NO_PMR)
  using allocator_type = pmr::polymorphic_allocator<void>;
  allocator_type get_allocator() const {return this->handler.get_allocator();}
#endif

  using cancellation_slot_type = asio::cancellation_slot;
  cancellation_slot_type get_cancellation_slot() const noexcept {return this->handler.get_cancellation_slot();}

  void get_return_object() {}
  std::suspend_never initial_suspend()        {return {};}

  auto final_suspend() noexcept
  {
    struct awaitable
    {
      completion_handler<Args...> & handler;
      constexpr bool await_ready() noexcept
      {
        return handler.completed_immediately != nullptr
           && *handler.completed_immediately == detail::completed_immediately_t::maybe;
      }

      std::coroutine_handle<void> await_suspend(std::coroutine_handle<composed_promise> h) noexcept
      {
        auto tmp = handler.self.release();
        detail::self_destroy(h);
        return tmp;
      }

      void await_resume() noexcept
      {
        *handler.completed_immediately = detail::completed_immediately_t::yes;
        handler.self.release();
      }

    };

    return awaitable{this->handler};
  }

  void unhandled_exception() {throw;}

};

template<typename ... Args>
composed_promise<Args...> completion_handler_probe(const completion_handler<Args...> & );

template<typename T>
concept is_completion_handler = requires (const T & h) {{completion_handler_probe(h)};};

}


namespace std
{

template<typename ... Args>
  requires  boost::cobalt::detail::is_completion_handler<
      boost::mp11::mp_back<boost::mp11::mp_list<Args...>>>
struct coroutine_traits<void, Args...>
{
  using promise_type = decltype(completion_handler_probe(
      std::declval<const boost::mp11::mp_back<boost::mp11::mp_list<Args...>>&>()));

};


}



#endif //BOOST_COBALT_COMPOSED_HPP
