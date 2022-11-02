// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_IMPL_CORO_HPP
#define CORO_IMPL_CORO_HPP

#include <asio/dispatch.hpp>
#include <asio/post.hpp>
#include <asio/prepend.hpp>

#include <coro/allocator.hpp>
#include <coro/concepts.hpp>
#include <coro/detail/wrapper.hpp>
#include <coro/async_operation.hpp>
#include <coro/ops.hpp>
#include <coro/this_coro.hpp>
#include <coro/util.hpp>
#include <asio/bind_allocator.hpp>

namespace coro
{

template <yield_signature Yield, typename Return, typename Executor, typename Allocator>
struct basic_coro;

namespace detail {

template <typename Signature, typename Return, typename Executor, typename Allocator>
struct coro_promise;

template <typename T>
struct is_noexcept : std::false_type
{
};

template <typename Return, typename... Args>
struct is_noexcept<Return(Args...)> : std::false_type
{
};

template <typename Return, typename... Args>
struct is_noexcept<Return(Args...) noexcept> : std::true_type
{
};

template <typename T>
constexpr bool is_noexcept_v = is_noexcept<T>::value;

struct coro_error
{
    static std::exception_ptr invalid()
    {
        return std::make_exception_ptr(asio::system_error(asio::error::fault));
    }

    static std::exception_ptr done()
    {
        return std::make_exception_ptr(asio::system_error(asio::error::broken_pipe));
    }
};

template <typename T, typename Coroutine >
struct coro_with_arg
{
    using coro_t = Coroutine;
    T value;
    coro_t& coro;

    struct awaitable_t
    {
        T value;
        coro_t& coro;

        alignas(sizeof(void*)) char buffer[1024];
        std::pmr::monotonic_buffer_resource resource{buffer, sizeof(buffer)};
        asio::cancellation_signal dispatched_signal;

        constexpr static bool await_ready() { return false; }

        template<typename Promise>
        auto direct_suspend(std::coroutine_handle<Promise> h)
        {
            auto & callee = *coro.coro_;
            auto & caller = h.promise();
            callee.awaited_from = h;
            callee.input_ = std::move(value);
            if constexpr (requires {{caller.get_cancellation_slot()};})
                callee.reset_cancellation_source(caller.get_cancellation_slot());
            return callee.get_handle();
        }

        template<typename Promise>
        auto dispatching_suspend(std::coroutine_handle<Promise> h)
        {
            auto & callee = *coro.coro_;
            auto & caller = h.promise();
            callee.awaited_from =
                    dispatch_coroutine(
                        asio::prefer(caller.get_executor(), asio::execution::outstanding_work.tracked),
                        asio::bind_allocator(
                                std::pmr::polymorphic_allocator<void>(&resource),
                                [h]() mutable { h.resume(); }));

            callee.reset_error();
            callee.input_ = std::move(value);

            if constexpr (requires {{caller.get_cancellation_slot()};})
            {
                callee.reset_cancellation_source(dispatched_signal.slot());
                if (caller.cancellation_state().slot().is_connected())
                {
                    struct cancel_handler
                    {
                        coro_t& coro;
                        asio::cancellation_signal  & dispatched_signal;
                        cancel_handler(coro_t& coro, asio::cancellation_signal & dispatched_signal)
                                : coro(coro), dispatched_signal(dispatched_signal) {}

                        void operator()(asio::cancellation_type ct)
                        {
                            asio::dispatch(
                                    coro.get_executor(),
                                    [this, ct]
                                    {
                                        dispatched_signal.emit(ct);
                                    });
                        }
                    };
                    caller.get_cancellation_slot().template emplace<cancel_handler>(coro, dispatched_signal);
                }
            }


            auto hh = std::coroutine_handle<typename coro_t::promise_type>::from_promise(*coro.coro_);
            return dispatch_coroutine(
                    coro.coro_->get_executor(),
                    asio::bind_allocator(
                            std::pmr::polymorphic_allocator<void>(&resource),
                            [hh]() mutable { hh.resume(); }));
        }

        template <typename Promise>
        auto await_suspend(std::coroutine_handle<Promise> h) -> std::coroutine_handle<>
        {
            auto & callee = *coro.coro_;
            auto & caller = h.promise();
            if constexpr (requires {{caller.get_executor()};})
            {
                if (caller.get_executor() == callee.get_executor()) // also if void & if vol
                    return direct_suspend(h);
                else
                    return dispatching_suspend(h);
            }
            else
                return direct_suspend(h);
        }

        auto await_resume() -> typename coro_t::result_type
        {
            coro.coro_->reset_cancellation_source(asio::cancellation_slot());
            coro.coro_->rethrow_if();
            return std::move(coro.coro_->result_);
        }
    };

    template <typename CompletionToken>
    auto async_resume(CompletionToken&& token) &&
    {
        return coro.async_resume(std::move(value),
                                 std::forward<CompletionToken>(token));
    }

    auto operator co_await() &&
    {
        return awaitable_t{std::move(value), coro};
    }
};

template <bool IsNoexcept>
struct coro_promise_error;

template <>
struct coro_promise_error<false>
{
    std::exception_ptr error_;

    void unhandled_exception()
    {
        error_ = std::current_exception();
    }

    void reset_error()
    {
        error_ = nullptr;
    }

    void rethrow_if()
    {
        if (error_)
            std::rethrow_exception(std::exchange(error_, nullptr));
    }
};

#if defined(__GNUC__)
# pragma GCC diagnostic push
# if defined(__clang__)
#  pragma GCC diagnostic ignored "-Wexceptions"
# else
#  pragma GCC diagnostic ignored "-Wterminate"
# endif
#elif defined(_MSC_VER)
# pragma warning(push)
# pragma warning (disable:4297)
#endif

template <>
struct coro_promise_error<true>
{
    void unhandled_exception() noexcept
    {
        throw;
    }
    void reset_error() {}
    void rethrow_if()
    {
    }
};

#if defined(__GNUC__)
# pragma GCC diagnostic pop
#elif defined(_MSC_VER)
# pragma warning(pop)
#endif

template <typename T = void>
struct yield_input
{
    T& value;
    std::coroutine_handle<> awaited_from{std::noop_coroutine()};

    bool await_ready() const noexcept
    {
        return false;
    }

    template <typename U>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<U>) noexcept
    {
        return std::exchange(awaited_from, std::noop_coroutine());
    }

    T await_resume() const noexcept
    {
        return std::move(value);
    }
};

template <>
struct yield_input<void>
{
    std::coroutine_handle<> awaited_from{std::noop_coroutine()};

    bool await_ready() const noexcept
    {
        return false;
    }

    auto await_suspend(std::coroutine_handle<>) noexcept
    {
        return std::exchange(awaited_from, std::noop_coroutine());
    }

    constexpr void await_resume() const noexcept
    {
    }
};

struct coro_awaited_from
{
    std::coroutine_handle<> awaited_from{std::noop_coroutine()};

    auto final_suspend() noexcept
    {
        struct suspendor
        {
            std::coroutine_handle<> awaited_from;

            constexpr static bool await_ready() noexcept
            {
                return false;
            }

            auto await_suspend(std::coroutine_handle<>) noexcept
            {
                return std::exchange(awaited_from, std::noop_coroutine());
            }

            constexpr static void await_resume() noexcept
            {
            }
        };

        return suspendor{std::exchange(awaited_from, std::noop_coroutine())};
    }

    ~coro_awaited_from()
    {
        awaited_from.resume();
    }//must be on the right executor
};

template <typename Yield, typename Input, typename Return>
struct coro_promise_exchange : coro_awaited_from
{
    using result_type = coro_result_t<Yield, Return>;

    result_type result_;
    Input input_;

    auto await_transform(this_coro::initial_t)
    {
        struct aw
        {
            Input & input_;
            bool await_ready() const noexcept
            {
                return true;
            }

            auto await_suspend(std::coroutine_handle<void> h) noexcept {return h;}

            Input await_resume() const noexcept
            {
                return std::move(input_);
            }
        };
        return aw{input_};
    }

    auto yield_value(Yield&& y)
    {
        result_ = std::move(y);
        return yield_input<Input>{std::move(input_),
                                  std::exchange(awaited_from, std::noop_coroutine())};
    }

    auto yield_value(const Yield& y)
    {
        result_ = y;
        return yield_input<Input>{std::move(input_),
                                  std::exchange(awaited_from, std::noop_coroutine())};
    }

    void return_value(const Return& r)
    {
        result_ = r;
    }

    void return_value(Return&& r)
    {
        result_ = std::move(r);
    }
};

template <typename YieldReturn>
struct coro_promise_exchange<YieldReturn, void, YieldReturn> : coro_awaited_from
{
    using result_type = coro_result_t<YieldReturn, YieldReturn>;

    result_type result_;

    auto await_transform(this_coro::initial_t) = delete;

    auto yield_value(const YieldReturn& y)
    {
        result_ = y;
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    auto yield_value(YieldReturn&& y)
    {
        result_ = std::move(y);
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    void return_value(const YieldReturn& r)
    {
        result_ = r;
    }

    void return_value(YieldReturn&& r)
    {
        result_ = std::move(r);
    }
};

template <typename Yield, typename Return>
struct coro_promise_exchange<Yield, void, Return> : coro_awaited_from
{
    using result_type = coro_result_t<Yield, Return>;

    result_type result_;

    auto await_transform(this_coro::initial_t) = delete;


    auto yield_value(const Yield& y)
    {
        result_.template emplace<0>(y);
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    auto yield_value(Yield&& y)
    {
        result_.template emplace<0>(std::move(y));
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    void return_value(const Return& r)
    {
        result_.template emplace<1>(r);
    }

    void return_value(Return&& r)
    {
        result_.template emplace<1>(std::move(r));
    }
};

template <typename Yield, typename Input>
struct coro_promise_exchange<Yield, Input, void> : coro_awaited_from
{
    using result_type = coro_result_t<Yield, void>;

    result_type result_;
    Input input_;

    auto await_transform(this_coro::initial_t)
    {
        struct aw
        {
            Input & input_;
            bool await_ready() const noexcept
            {
                return true;
            }

            auto await_suspend(std::coroutine_handle<void> h) noexcept {return h;}

            Input await_resume() const noexcept
            {
                return std::move(input_);
            }
        };
        return aw{input_};
    }

    auto yield_value(Yield&& y)
    {
        result_ = std::move(y);
        return yield_input<Input>{input_,
                                  std::exchange(awaited_from, std::noop_coroutine())};
    }

    auto yield_value(const Yield& y)
    {
        result_ = y;
        return yield_input<Input>{input_,
                                  std::exchange(awaited_from, std::noop_coroutine())};
    }

    void return_void()
    {
        result_.reset();
    }
};

template <typename Return>
struct coro_promise_exchange<void, void, Return> : coro_awaited_from
{
    using result_type = coro_result_t<void, Return>;

    result_type result_;

    auto await_transform(this_coro::initial_t) = delete;
    void yield_value() = delete;

    void return_value(const Return& r)
    {
        result_ = r;
    }

    void return_value(Return&& r)
    {
        result_ = std::move(r);
    }
};

template <>
struct coro_promise_exchange<void, void, void> : coro_awaited_from
{
    void return_void() {}
    auto await_transform(this_coro::initial_t) = delete;
    void yield_value() = delete;
};

template <typename Yield>
struct coro_promise_exchange<Yield, void, void> : coro_awaited_from
{
    using result_type = coro_result_t<Yield, void>;

    result_type result_;

    auto await_transform(this_coro::initial_t) = delete;

    auto yield_value(const Yield& y)
    {
        result_ = y;
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    auto yield_value(Yield&& y)
    {
        result_ = std::move(y);
        return yield_input<void>{std::exchange(awaited_from, std::noop_coroutine())};
    }

    void return_void()
    {
        result_.reset();
    }
};

template <typename Yield,
          typename Return,
          typename Executor,
          typename Allocator>
struct coro_promise final :
        promise_allocator_arg_base<Allocator>,
        promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>,
        promise_throw_if_cancelled_base,
        promise_executor_base<Executor>,
        enable_awaitables<coro_promise<Yield, Return, Executor, Allocator>>,
        enable_operations<>,
        enable_await_allocator<Allocator>,
        coro_promise_error<coro_traits<Yield, Return, Executor>::is_noexcept>,
        coro_promise_exchange<
                typename coro_traits<Yield, Return, Executor>::yield_type,
                typename coro_traits<Yield, Return, Executor>::input_type,
                typename coro_traits<Yield, Return, Executor>::return_type>
{
    using coro_type = basic_coro<Yield, Return, Executor>;

    auto handle()
    {
        return std::coroutine_handle<coro_promise>::from_promise(this);
    }

    using traits = coro_traits<Yield, Return, Executor>;

    using input_type = typename traits::input_type;
    using yield_type = typename traits::yield_type;
    using return_type = typename traits::return_type;
    using error_type = typename traits::error_type;
    using result_type = typename traits::result_type;
    constexpr static bool is_noexcept = traits::is_noexcept;


    auto get_handle()
    {
        return std::coroutine_handle<coro_promise>::from_promise(*this);
    }

    template <typename... Args>
    coro_promise(Executor executor, Args&&...args) noexcept
            : promise_allocator_arg_base<Allocator>(executor, args...),
              promise_executor_base<Executor>(std::move(executor))
    {
    }

    template <typename First, typename... Args>
    coro_promise(First&& f, Executor executor, Args&&... args) noexcept
            : promise_allocator_arg_base<Allocator>(f, executor, args...),
              promise_executor_base<Executor>(std::move(executor))
    {
    }

    template <typename First, detail::with_get_executor Context, typename... Args>
    coro_promise(First&& f, Context&& ctx, Args&&... args) noexcept
            : promise_allocator_arg_base<Allocator>(f, ctx, args...),
              promise_executor_base<Executor>(ctx.get_executor())
    {
    }

    template <detail::with_get_executor Context, typename... Args>
    coro_promise(Context&& ctx, Args&&... args) noexcept
            : promise_allocator_arg_base<Allocator>(ctx, args...),
              promise_executor_base<Executor>(ctx.get_executor())
    {
    }

    auto get_return_object()
    {
        return basic_coro<Yield, Return, Executor>{this};
    }

    auto initial_suspend() noexcept
    {
        return std::suspend_always{};
    }

    using coro_promise_exchange<
            typename coro_traits<Yield, Return, Executor>::yield_type,
            typename coro_traits<Yield, Return, Executor>::input_type,
            typename coro_traits<Yield, Return, Executor>::return_type>::yield_value;

    using coro_promise_exchange<
            typename coro_traits<Yield, Return, Executor>::yield_type,
            typename coro_traits<Yield, Return, Executor>::input_type,
            typename coro_traits<Yield, Return, Executor>::return_type>::await_transform;

    using enable_await_allocator<Allocator>::await_transform;
    using promise_cancellation_base<asio::cancellation_slot, asio::enable_total_cancellation>::await_transform;
    using promise_throw_if_cancelled_base::await_transform;
    using enable_awaitables<coro_promise<Yield, Return, Executor, Allocator>>::await_transform;
    using enable_operations<>::await_transform;
    using promise_executor_base<Executor>::await_transform;
};

} // namespace detail

template <yield_signature Yield, typename Return, typename Executor, typename Allocator>
struct basic_coro<Yield, Return, Executor, Allocator>::awaitable_t
{
    using coro_t = basic_coro;
    basic_coro& coro;
    alignas(sizeof(void*)) char buffer[1024];
    std::pmr::monotonic_buffer_resource resource{buffer, sizeof(buffer)};
    asio::cancellation_signal dispatched_signal;

    constexpr static bool await_ready() { return false; }

    template<typename Promise>
    auto direct_suspend(std::coroutine_handle<Promise> h)
    {
        auto & callee = *coro.coro_;
        auto & caller = h.promise();
        callee.awaited_from = h;
        if constexpr (requires {{caller.get_cancellation_slot()};})
            callee.reset_cancellation_source(caller.get_cancellation_slot());
        return callee.get_handle();
    }

    template<typename Promise>
    auto dispatching_suspend(std::coroutine_handle<Promise> h)
    {
        auto & callee = *coro.coro_;
        auto & caller = h.promise();
        callee.awaited_from =
                detail::dispatch_coroutine(
                        asio::prefer(caller.get_executor(), asio::execution::outstanding_work.tracked),
                        asio::bind_allocator(
                                std::pmr::polymorphic_allocator<void>(&resource),
                                [h]() mutable { h.resume(); }));

        callee.reset_error();

        if constexpr (requires {{caller.get_cancellation_slot()};})
        {
            callee.reset_cancellation_source(dispatched_signal.slot());
            if (caller.get_cancellation_slot().is_connected())
            {
                struct cancel_handler
                {
                    coro_t& coro;
                    asio::cancellation_signal  & dispatched_signal;
                    cancel_handler(coro_t& coro, asio::cancellation_signal & dispatched_signal)
                            : coro(coro), dispatched_signal(dispatched_signal) {}

                    void operator()(asio::cancellation_type ct)
                    {
                        asio::dispatch(
                                coro.get_executor(),
                                [this, ct]
                                {
                                    dispatched_signal.emit(ct);
                                });
                    }
                };
                caller.get_cancellation_slot().template emplace<cancel_handler>(coro, dispatched_signal);
            }
        }


        auto hh = std::coroutine_handle<typename coro_t::promise_type>::from_promise(*coro.coro_);
        return detail::dispatch_coroutine(
                coro.coro_->get_executor(),
                asio::bind_allocator(
                        std::pmr::polymorphic_allocator<void>(&resource),
                        [hh]() mutable { hh.resume(); }));
    }


    template <typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) -> std::coroutine_handle<>
    {
        auto & callee = *coro.coro_;
        auto & caller = h.promise();
        if constexpr (requires {{caller.get_executor()};})
        {
            if (caller.get_executor() == callee.get_executor()) // also if void & if vol
                return direct_suspend(h);
            else
                return dispatching_suspend(h);
        }
        else
            return direct_suspend(h);
    }

    template <typename Y, typename R, typename E, typename A>
    auto await_suspend(
            std::coroutine_handle<detail::coro_promise<Y, R, E, A>> h)
    -> std::coroutine_handle<>
    {
        using callee_promise = detail::coro_promise<Y, R, E, A>;
        auto & caller = h.promise();
        detail::throw_if_cancelled_impl(caller);
        auto & callee = *coro.coro_;

        if (caller.get_executor() == callee.get_executor()) // also if void & if vol
        {
            callee.awaited_from = h;
            callee.reset_cancellation_source(caller.cancellation_state().slot());
            return callee.get_handle();
        }
        else
        {
            callee.awaited_from =
                    detail::dispatch_coroutine(
                            asio::prefer(caller.get_executor(), asio::execution::outstanding_work.tracked),
                            asio::bind_allocator(
                                    std::pmr::polymorphic_allocator<void>(&resource),
                                    [h]() mutable { h.resume(); }));

            callee.reset_error();
            callee.reset_cancellation_source(dispatched_signal.slot());
            if (caller.cancellation_state().slot().is_connected())
            {
                struct cancel_handler
                {
                    coro_t& coro;
                    asio::cancellation_signal  & dispatched_signal;
                    cancel_handler(coro_t& coro, asio::cancellation_signal & dispatched_signal)
                            : coro(coro), dispatched_signal(dispatched_signal) {}

                    void operator()(asio::cancellation_type ct)
                    {
                        asio::dispatch(
                                coro.get_executor(),
                                [this, ct]
                                {
                                    dispatched_signal.emit(ct);
                                });
                    }
                };
                caller.cancellation_state().slot().template emplace<cancel_handler>(coro, dispatched_signal);
            }

            auto hh = std::coroutine_handle<typename coro_t::promise_type>::from_promise(*coro.coro_);
            return detail::dispatch_coroutine(
                    coro.coro_->get_executor(),
                    asio::bind_allocator(
                            std::pmr::polymorphic_allocator<void>(&resource),
                            [hh]() mutable { hh.resume(); }));
        }
    }

    auto await_resume() -> result_type
    {
        coro.coro_->reset_cancellation_source();
        coro.coro_->rethrow_if();
        coro.coro_->reset_cancellation_source(asio::cancellation_slot());
        if constexpr (!std::is_void_v<result_type>)
            return std::move(coro.coro_->result_);
    }
};

template <yield_signature Yield, typename Return, typename Executor, typename Allocator>
struct basic_coro<Yield, Return, Executor, Allocator>::initiate_async_resume
{
    typedef Executor executor_type;
    typedef Allocator allocator_type;

    explicit initiate_async_resume(basic_coro::promise_type* self)
            : coro_(self)
    {
    }

    executor_type get_executor() const noexcept
    {
        return coro_->get_executor();
    }

    allocator_type get_allocator() const noexcept
    {
        return coro_->get_allocator();
    }

    template <typename E, typename WaitHandler>
    auto handle(E exec, WaitHandler&& handler,
                std::true_type /* error is noexcept */,
                std::true_type /* result is void */)  //noexcept
    {
        return [this, coro = coro_,
                h = std::forward<WaitHandler>(handler),
                exec = std::move(exec)]() mutable
        {
            assert(coro);

            auto ch = std::coroutine_handle<promise_type>::from_promise(*coro);
            assert(ch && !ch.done());

            coro->awaited_from = post_coroutine(std::move(exec), std::move(h));
            coro->reset_error();
            ch.resume();
        };
    }

    template <typename E, typename WaitHandler>
    requires (!std::is_void_v<result_type>)
    auto handle(E exec, WaitHandler&& handler,
                std::true_type /* error is noexcept */,
                std::false_type  /* result is void */)  //noexcept
    {
        return [coro = coro_,
                h = std::forward<WaitHandler>(handler),
                exec = std::move(exec)]() mutable
        {
            assert(coro);

            auto ch = std::coroutine_handle<promise_type>::from_promise(*coro);
            assert(ch && !ch.done());

            coro->awaited_from = detail::post_coroutine(exec,
                                                        asio::bind_allocator(ch.promise().get_allocator(),
                                                            [coro, h = std::move(h)]() mutable
                                                            {
                                                                std::move(h)(std::move(coro->result_));
                                                            }));
            coro->reset_error();
            ch.resume();
        };
    }

    template <typename E, typename WaitHandler>
    auto handle(E exec, WaitHandler&& handler,
                std::false_type /* error is noexcept */,
                std::true_type /* result is void */)
    {
        return [coro = coro_,
                h = std::forward<WaitHandler>(handler),
                exec = std::move(exec)]() mutable
        {
            if (!coro)
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::invalid()));
            auto ch = std::coroutine_handle<promise_type>::from_promise(*coro);
            if (!ch)
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::invalid()));
            else if (ch.done())
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::done()));
            else
            {
                assert(coro->awaited_from == std::noop_coroutine());
                coro->awaited_from =
                        detail::post_coroutine(exec,
                            asio::bind_allocator(
                              asio::get_associated_allocator(h, coro->get_allocator()),
                               [coro, h = std::move(h)]() mutable
                               {
                                 std::move(h)(std::move(coro->error_));
                               }));
                coro->reset_error();
                ch.resume();
            }
        };
    }

    template <typename E, typename WaitHandler>
    auto handle(E exec, WaitHandler&& handler,
                std::false_type /* error is noexcept */,
                std::false_type  /* result is void */)
    {
        return [coro = coro_,
                h = std::forward<WaitHandler>(handler),
                exec = std::move(exec)]() mutable
        {
            if (!coro)
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::invalid(), result_type{}));

            auto ch =
                    std::coroutine_handle<promise_type>::from_promise(*coro);
            if (!ch)
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::invalid(), result_type{}));
            else if (ch.done())
                return asio::post(exec, asio::prepend(std::move(h), detail::coro_error::done(), result_type{}));
            else
            {
                assert(coro->awaited_from == std::noop_coroutine());
                coro->awaited_from =
                        detail::post_coroutine(exec,
                                               asio::bind_allocator(
                                                       asio::get_associated_allocator(h, coro->get_allocator()),
                                                       [h = std::move(h), coro]() mutable
                                               {
                                                   std::move(h)(
                                                           std::move(coro->error_),
                                                           std::move(coro->result_));
                                               }));
                coro->reset_error();
                ch.resume();
            }
        };
    }

    template <typename WaitHandler>
    void operator()(WaitHandler&& handler)
    {
        const auto exec = asio::prefer(
                get_associated_executor(handler, get_executor()),
                asio::execution::outstanding_work.tracked);

        coro_->reset_cancellation_source(asio::get_associated_cancellation_slot(handler));
        asio::dispatch(get_executor(),
                       handle(exec, std::forward<WaitHandler>(handler),
                              std::integral_constant<bool, is_noexcept>{},
                              std::is_void<result_type>{}));
    }

    template <typename WaitHandler, typename Input>
    void operator()(WaitHandler&& handler, Input&& input)
    {
        const auto exec = asio::prefer(
                get_associated_executor(handler, get_executor()),
                asio::execution::outstanding_work.tracked);

        coro_->reset_cancellation_source(asio::get_associated_cancellation_slot(handler));
        asio::dispatch(get_executor(),
                       asio::bind_allocator(
                           asio::get_associated_allocator(handler, get_allocator()),
                           [h = handle(exec, std::forward<WaitHandler>(handler),
                                       std::integral_constant<bool, is_noexcept>{},
                                       std::is_void<result_type>{}),
                                   in = std::forward<Input>(input), coro = coro_]() mutable
                           {
                               coro->input_ = std::move(in);
                               std::move(h)();
                           }));
    }

  private:
    typename basic_coro::promise_type* coro_;
};


}

#endif //CORO_IMPL_CORO_HPP
