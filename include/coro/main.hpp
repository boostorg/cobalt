// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_MAIN_HPP
#define CORO_MAIN_HPP

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/signal_set.hpp>

#include <coroutine>
#include <memory_resource>
#include <optional>

#include <coro/concepts.hpp>
#include <coro/allocator.hpp>
#include <coro/async_operation.hpp>
#include <coro/ops.hpp>
#include <coro/this_coro.hpp>

namespace asio
{
struct thread_pool;
struct system_context;

}

namespace coro
{

namespace detail
{

template<typename Context = asio::io_context>
struct basic_main_promise;
}

template<typename Context = asio::io_context>
struct basic_main { detail::basic_main_promise<Context> * promise; };

using main = basic_main<>;
using threaded_main = basic_main<asio::thread_pool>;
using system_main   = basic_main<asio::system_context>;

}

auto co_main         (int argc, char * argv[]) -> coro::main;
auto co_threaded_main(int argc, char * argv[]) -> coro::threaded_main;
auto co_system_main  (int argc, char * argv[]) -> coro::system_main;



namespace coro {

namespace detail
{
extern "C"
{
int main(int argc, char * argv[]);
}

struct signal_helper
{
    asio::cancellation_signal signal;
};

inline void run_impl(asio::io_context & ctx) {ctx.run();}

template<typename Pool, void (Pool::*)() = &Pool::join>
void run_impl(Pool & tp) {tp.join();}

template<typename Context>
struct basic_main_promise : signal_helper,
                      promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
                      promise_throw_if_cancelled_base,
                      enable_awaitables<basic_main_promise<Context>>,
                      enable_async_operation,
                      enable_await_allocator<basic_main_promise<Context>>
{
    basic_main_promise(int, char **) : promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>(
            signal_helper::signal.slot(), asio::enable_total_cancellation())
    {
        [[maybe_unused]] volatile auto p = &detail::main;
    }

    static std::pmr::memory_resource * my_resource;
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

    friend auto ::co_main         (int argc, char * argv[]) -> coro::main;
    friend auto ::co_threaded_main(int argc, char * argv[]) -> coro::threaded_main;
    friend auto ::co_system_main  (int argc, char * argv[]) -> coro::system_main;

    static auto call_co_main(int argc, char * argv[], asio::io_context &) -> coro::main
    {
        return co_main(argc, argv);
    }
    static auto call_co_main(int argc, char * argv[], asio::thread_pool &) -> coro::threaded_main
    {
        return co_threaded_main(argc, argv);
    }

    static auto call_co_main(int argc, char * argv[], asio::system_context &) -> coro::system_main
    {
        return co_system_main(argc, argv);
    }


    friend int main(int argc, char * argv[])
    {
        std::conditional_t<std::is_same_v<Context, asio::io_context>,
                           std::pmr::unsynchronized_pool_resource,
                           std::pmr::synchronized_pool_resource> root_resource;
        coro::set_default_resource(&root_resource);

        char buffer[8096];
        std::pmr::monotonic_buffer_resource main_res{buffer, 8096, &root_resource};
        my_resource = &main_res;

        Context ctx;

        ::coro::basic_main<Context> mn = call_co_main(argc, argv, ctx);
        int res ;
        mn.promise->result = &res;
        mn.promise->exec.emplace(ctx.get_executor());
        auto p = std::coroutine_handle<detail::basic_main_promise<Context>>::from_promise(*mn.promise);
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

        run_impl(ctx);
        return res;
    }

    using executor_type = typename Context::executor_type;
    executor_type get_executor() const {return exec->get_executor();}

    using allocator_type = std::pmr::polymorphic_allocator<void>;
    using resource_type  = std::conditional_t<std::is_same_v<Context, asio::io_context>,
                                              std::pmr::synchronized_pool_resource,
                                              std::pmr::unsynchronized_pool_resource>;

    resource_type resource;
    allocator_type  get_allocator() { return allocator_type(&resource); }

    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<basic_main_promise>::await_transform;
    using enable_async_operation::await_transform;
    using enable_await_allocator<basic_main_promise<Context>>::await_transform;

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
    ::coro::basic_main<Context> get_return_object()
    {
        return ::coro::basic_main<Context>{this};
    }
};

template<typename Context>
std::pmr::memory_resource *  basic_main_promise<Context>::my_resource = std::pmr::get_default_resource();

}

}

namespace std
{

template<typename Context>
struct coroutine_traits<coro::basic_main<Context>, int, char**>
{
    using promise_type = coro::detail::basic_main_promise<Context>;
};

}


#endif //CORO_MAIN_HPP
