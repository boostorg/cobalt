// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ASYNC_MAIN_HPP
#define BOOST_ASYNC_MAIN_HPP

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>

#include <coroutine>
#include <memory_resource>
#include <optional>

#include <boost/async/concepts.hpp>
#include <boost/async/async_operation.hpp>
#include <boost/async/this_coro.hpp>


namespace boost::async
{

namespace detail
{
struct main_promise;
}

struct main;

}

auto co_main(int argc, char * argv[]) -> boost::async::main;

namespace boost::async
{

namespace detail
{
extern "C"
{
int main(int argc, char * argv[]);
}
}

class main
{
  detail::main_promise * promise;
  main(detail::main_promise * promise) : promise(promise) {}
  friend auto ::co_main(int argc, char * argv[]) -> boost::async::main;
  friend int detail::main(int argc, char * argv[]);
  friend struct detail::main_promise;
};

namespace detail
{
struct signal_helper
{
    asio::cancellation_signal signal;
};

struct main_promise : signal_helper,
                      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                      promise_throw_if_cancelled_base,
                      enable_awaitables<main_promise>,
                      enable_async_operation,
                      enable_await_allocator<main_promise>
{
    main_promise(int, char **) : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
            signal_helper::signal.slot(), asio::enable_total_cancellation())
    {
        [[maybe_unused]] volatile auto p = &detail::main;
    }

    inline static std::pmr::memory_resource * my_resource = std::pmr::get_default_resource();
    void * operator new(const std::size_t size)
    {
        return my_resource->allocate(size);
    }

    void operator delete(void * raw, const std::size_t size)
    {
        return my_resource->deallocate(raw, size);
    }

    std::suspend_always initial_suspend() {return {};}
    auto final_suspend() noexcept
    {
        asio::error_code ec;
        if (signal_set)
            signal_set->cancel(ec);
        return std::suspend_never(); // enable_yielding_tasks::final_suspend();
    }

    void unhandled_exception() { throw ; }
    void return_value(int res = 0)
    {
        if (result)
            *result = res;
    }

    friend auto ::co_main (int argc, char * argv[]) -> boost::async::main;


    friend int main(int argc, char * argv[])
    {
        std::pmr::unsynchronized_pool_resource root_resource;
        auto pre = boost::async::this_thread::set_default_resource(&root_resource);
        char buffer[8096];
        std::pmr::monotonic_buffer_resource main_res{buffer, 8096, &root_resource};
        my_resource = &main_res;

        asio::io_context ctx;
        boost::async::this_thread::set_executor(ctx.get_executor());
        ::boost::async::main mn = co_main(argc, argv);
        int res ;
        mn.promise->result = &res;
        mn.promise->exec.emplace(ctx.get_executor());
        auto p = std::coroutine_handle<detail::main_promise>::from_promise(*mn.promise);
        asio::signal_set ss{ctx, SIGINT, SIGTERM};
        mn.promise->signal_set = &ss;
        struct work
        {
            asio::signal_set & ss;
            asio::cancellation_signal & signal;
            void operator()(asio::error_code ec, int sig) const
            {
                if (sig == SIGINT)
                    signal.emit(asio::cancellation_type::total);
                if (sig == SIGTERM)
                    signal.emit(asio::cancellation_type::terminal);
                if (!ec)
                    ss.async_wait(*this);
            }
        };

        ss.async_wait(work{ss, mn.promise->signal});
        asio::post(ctx.get_executor(), [p]{p.resume();});

        ctx.run();
        return res;
    }

    using executor_type = typename asio::io_context::executor_type;
    executor_type get_executor() const {return exec->get_executor();}

    using allocator_type = std::pmr::polymorphic_allocator<void>;
    using resource_type  = std::pmr::unsynchronized_pool_resource;

    mutable resource_type resource{my_resource};
    allocator_type get_allocator() const { return allocator_type(&resource); }

    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<main_promise>::await_transform;
    using enable_async_operation::await_transform;
    using enable_await_allocator<main_promise>::await_transform;

    auto await_transform(this_coro::executor_t) const
    {
        struct exec_helper
        {
            executor_type value;

            constexpr static bool await_ready() noexcept
            {
                return true;
            }

            constexpr static void await_suspend(std::coroutine_handle<>) noexcept
            {
            }

            executor_type await_resume() const noexcept
            {
                return value;
            }
        };

        return exec_helper{get_executor()};
    }

  private:
    int * result;
    std::optional<asio::executor_work_guard<executor_type>> exec;
    asio::signal_set * signal_set;
    ::boost::async::main get_return_object()
    {
        return ::boost::async::main{this};
    }
};

}

}

namespace std
{

template<>
struct coroutine_traits<boost::async::main, int, char**>
{
    using promise_type = boost::async::detail::main_promise;
};

}


#endif //BOOST_ASYNC_MAIN_HPP
