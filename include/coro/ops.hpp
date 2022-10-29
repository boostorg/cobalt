// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_OPS_HPP
#define CORO_OPS_HPP

#include <tuple>
#include <coro/async_operation.hpp>
#include <asio/read.hpp>
#include <asio/read_at.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/write.hpp>
#include <asio/write_at.hpp>

namespace asio::experimental
{

template<
        typename Executor,
        typename Traits,
        typename... Signatures>
class basic_concurrent_channel;

template<
        typename Executor,
        typename Traits,
        typename... Signatures>
class basic_channel;

}

namespace coro
{


template<typename T>
concept operation = requires (T op, asio::error_code & ec) {op();} && async_operation<T>;

namespace ops
{

template<typename Op> struct throwing : Op { using Op::Op; };
template<typename Op> struct returned : Op { using Op::Op; };


namespace detail
{

struct intercept_error
{
    asio::error_code & ec;
    intercept_error(asio::error_code & ec) : ec(ec) {}
    template<typename ... Args>
    auto operator()(asio::error_code ec_, Args && ... args) const
    {
        this->ec = ec_;
        return asio::deferred.values(std::forward<Args>(args)...);
    }
};

template<typename Func, typename Tuple, std::size_t ... Idx>
auto apply_with_last_at_front_impl(Func && func, Tuple && tup, std::index_sequence<Idx...>)
{
    return std::forward<Func>(func)(
            std::get<sizeof...(Idx)>(std::forward<Tuple>(tup)),
            std::get<Idx>(std::forward<Tuple>(tup))...
            );
}


template<typename Func, typename Tuple>
auto apply_with_last_at_front(Func && func, Tuple && tup)
{
    return apply_with_last_at_front_impl(std::forward<Func>(func), std::forward<Tuple>(tup),
                                         std::make_index_sequence<
                                         std::tuple_size_v<std::decay_t<Tuple>> - 1>());
}

};

#define CORO_DEFINE_OPERATION(Scope, Name)                                                                             \
template<typename ... Args>                                                                                            \
struct Name                                                                                                            \
{                                                                                                                      \
    std::tuple<Args ... > args;                                                                                        \
    explicit Name(Args && ... args) : args(std::forward<Args>(args)...) {}                                             \
                                                                                                                       \
    std::size_t operator()() &&                                                                                        \
    {                                                                                                                  \
        return std::apply(                                                                                             \
                []<typename ... Args_>(Args_ && ... args_)                                                             \
                {                                                                                                      \
                    return Scope::Name(std::forward<Args_>(args_)...);                                                 \
                }, std::move(args));                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    std::size_t operator()()  &                                                                                        \
    {                                                                                                                  \
        return std::apply(                                                                                             \
                []<typename ... Args_>(Args_ && ... args_)                                                             \
                {                                                                                                      \
                    return Scope::Name(std::forward<Args_>(args_)...);                                                 \
                }, args);                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    template<typename Token>                                                                                           \
    auto operator()(Token && tk) &&                                                                                    \
    {                                                                                                                  \
        if constexpr (sizeof...(Args) > 0 &&                                                                           \
                      std::is_same_v<variadic_last_t< Args...>, asio::error_code &>)                                   \
            return detail::apply_with_last_at_front(                                                                   \
                    [&]<typename ... Args_>(asio::error_code &ec, Args_ && ... args_)                                  \
                    {                                                                                                  \
                        return Scope::async_##Name(std::forward<Args_>(args_)...,                                      \
                                                   asio::deferred(detail::intercept_error(ec)))                        \
                                                   (std::forward<Token>(tk));                                          \
                    }, std::move(args));                                                                               \
        else                                                                                                           \
            return std::apply(                                                                                         \
                    [&]<typename ... Args_>(Args_ && ... args_)                                                        \
                    {                                                                                                  \
                        return Scope::async_##Name(std::forward<Args_>(args_)..., std::forward<Token>(tk));            \
                    }, std::move(args));                                                                               \
    }                                                                                                                  \
    template<typename Token>                                                                                           \
    auto operator()(Token && tk) &                                                                                     \
    {                                                                                                                  \
        if constexpr (sizeof...(Args) > 0 &&                                                                           \
                      std::is_same_v<variadic_last_t<Args...>, asio::error_code &>)                                    \
            return detail::apply_with_last_at_front(                                                                   \
                    [&]<typename ... Args_>(asio::error_code &ec, Args_ && ... args_)                                  \
                    {                                                                                                  \
                        return Scope::async_##Name(std::forward<Args_>(args_)...,                                      \
                                                   asio::deferred(detail::intercept_error(ec)))                        \
                                                   (std::forward<Token>(tk));                                          \
                    }, args);                                                                                          \
        else                                                                                                           \
            return std::apply(                                                                                         \
                    [&]<typename ... Args_>(Args_ && ... args_)                                                        \
                    {                                                                                                  \
                        return Scope::async_##Name(std::forward<Args_>(args_)..., std::forward<Token>(tk));            \
                    }, args);                                                                                          \
    }                                                                                                                  \
                                                                                                                       \
};                                                                                                                     \
                                                                                                                       \
template<typename ... Args>                                                                                            \
Name(Args && ... args) -> Name<Args...>;



#define CORO_DEFINE_MEMBER_OPERATION(Name)                                                                             \
template<typename Object, typename ... Args>                                                                           \
struct Name                                                                                                            \
{                                                                                                                      \
    Object & object;                                                                                                   \
    std::tuple<Args ... > args;                                                                                        \
    explicit Name(Object & object, Args && ... args) : object(object),  args(std::forward<Args>(args)...) {}           \
                                                                                                                       \
    auto operator()() &&                                                                                               \
    {                                                                                                                  \
        return std::apply(                                                                                             \
                [this]<typename ... Args_>(Args_ && ... args_)                                                         \
                {                                                                                                      \
                    return object.Name(std::forward<Args_>(args_)...);                                                 \
                }, std::move(args));                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    auto operator()() &                                                                                                \
    {                                                                                                                  \
        return std::apply(                                                                                             \
                [this]<typename ... Args_>(Args_ && ... args_)                                                         \
                {                                                                                                      \
                    return object.Name(std::forward<Args_>(args_)...);                                                 \
                }, std::move(args));                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template<typename Token>                                                                                           \
    auto operator()(Token && tk) &&                                                                                    \
    {                                                                                                                  \
        if constexpr (std::is_same_v<variadic_last_t<Args...>, asio::error_code &>)                                    \
            return detail::apply_with_last_at_front(                                                                   \
                [&]<typename ... Args_>(asio::error_code &ec, Args_ && ... args_)                                      \
                {                                                                                                      \
                    return object.async_##Name(std::forward<Args_>(args_)...,                                          \
                                               asio::deferred(detail::intercept_error(ec)))                            \
                                               (std::forward<Token>(tk));                                              \
                }, std::move(args));                                                                                   \
        else                                                                                                           \
            return std::apply(                                                                                         \
                [&]<typename ... Args_>(Args_ && ... args_)                                                            \
                {                                                                                                      \
                    return object.async_##Name(std::forward<Args_>(args_)..., std::forward<Token>(tk));                \
                }, std::move(args));                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template<typename Token>                                                                                           \
    auto operator()(Token && tk) &                                                                                     \
    {                                                                                                                  \
        if constexpr (std::is_same_v<variadic_last_t<Args...>, asio::error_code &>)                                    \
            return detail::apply_with_last_at_front(                                                                   \
                [&]<typename ... Args_>(asio::error_code &ec, Args_ && ... args_)                                      \
                {                                                                                                      \
                    return object.async_##Name(std::forward<Args_>(args_)...,                                          \
                                               asio::deferred(detail::intercept_error(ec)))                            \
                                               (std::forward<Token>(tk));                                              \
                }, args);                                                                                              \
        else                                                                                                           \
            return std::apply(                                                                                         \
                [&]<typename ... Args_>(Args_ && ... args_)                                                            \
                {                                                                                                      \
                    return object.async_##Name(std::forward<Args_>(args_)..., std::forward<Token>(tk));                \
                }, args);                                                                                              \
    }                                                                                                                  \
};                                                                                                                     \
                                                                                                                       \
template<typename Object, typename ... Args>                                                                           \
Name(Object & object, Args && ... args) -> Name<Object, Args...>;

CORO_DEFINE_OPERATION(asio, write);
CORO_DEFINE_OPERATION(asio, write_at);
CORO_DEFINE_OPERATION(asio, read);
CORO_DEFINE_OPERATION(asio, read_at);
CORO_DEFINE_OPERATION(asio, read_until);
CORO_DEFINE_MEMBER_OPERATION(wait);
CORO_DEFINE_MEMBER_OPERATION(read_some);
CORO_DEFINE_MEMBER_OPERATION(write_some);
CORO_DEFINE_MEMBER_OPERATION(send);
CORO_DEFINE_MEMBER_OPERATION(send_to);
CORO_DEFINE_MEMBER_OPERATION(receive);
CORO_DEFINE_MEMBER_OPERATION(receive_from);

CORO_DEFINE_MEMBER_OPERATION(connect);
CORO_DEFINE_MEMBER_OPERATION(accept);
CORO_DEFINE_MEMBER_OPERATION(close);

CORO_DEFINE_MEMBER_OPERATION(handshake);
CORO_DEFINE_MEMBER_OPERATION(shutdown);

CORO_DEFINE_MEMBER_OPERATION(read_header);
CORO_DEFINE_MEMBER_OPERATION(write_header);

CORO_DEFINE_MEMBER_OPERATION(ping);
CORO_DEFINE_MEMBER_OPERATION(pong);


namespace detail
{

template<typename Channel, typename ... Args>
struct channel_send
{
    Channel & channel;
    std::tuple<Args ... > args;
    std::optional<std::tuple<asio::error_code>> &result;
    explicit channel_send(Channel & channel, Args && ... args) : channel(channel),  args(std::forward<Args>(args)...) {}

    bool await_ready()
    {
        return std::apply([this](auto & ... vals){return channel.try_send(std::move(vals)...);}, args);
    }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        throw_if_cancelled_impl(h.promise());
        using completion = completion_handler<Promise, Args...>;
        if constexpr (std::is_same_v<variadic_last_t<Args...>, asio::error_code &>)
            detail::apply_with_last_at_front([&](asio::error_code & ec, auto && ...  args_){
                channel.async_send(std::move(args_)..., completion{h, result});
            });
        else
            std::apply([&](auto && ...  args_){
                channel.async_send(std::move(args_)..., completion{h, result});
            });
    }

    auto await_resume()
    {
        if (auto ec = std::get<0>(*result))
        {
            if constexpr (std::is_same_v<variadic_last_t<Args...>, asio::error_code &>)
                std::get<(sizeof...(Args) - 1)>(args) = ec;
            else
                throw asio::system_error(ec);
        }
    }
};



template<typename Channel, typename Sig, typename ... Args>
struct channel_receive;

template<typename Channel, typename ... Ts>
struct channel_receive<Channel, void(Ts...)>
{
    std::optional<std::tuple<Ts...>> &result;
    Channel & channel;
    explicit channel_receive(Channel & channel) : channel(channel) {}

    bool await_ready()
    {
        return channel.try_receive(
                [this](auto && ... args)
                {
                    result = {std::move(args)...};
                });
    }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        throw_if_cancelled_impl(h.promise());
        using completion = completion_handler<Promise, Ts...>;
        channel.async_receive(completion{h, result});
    }

    auto await_resume()
    {
        return interpret_result(std::move(*result));
    }
};


template<typename Channel, typename ... Ts>
struct channel_receive<Channel, void(asio::error_code, Ts...), asio::error_code & >
{
    std::optional<std::tuple<Ts...>> &result;
    Channel & channel;
    asio::error_code & ec;
    explicit channel_receive(Channel & channel, asio::error_code & ec) : channel(channel), ec(ec) {}

    bool await_ready()
    {
        return channel.try_receive(
                [this](asio::error_code & ec, auto && ... args)
                {
                    ec = ec;
                    result = {std::move(args)...};
                });
    }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        throw_if_cancelled_impl(h.promise());
        using completion = completion_handler<Promise, Ts...>;
        channel.async_receive(asio::redirect_error(completion{h, result}, ec));
    }

    auto await_resume()
    {
        return forward_result(std::move(*result));
    }
};


template<typename Channel, typename ... Ts>
struct channel_receive<Channel, void(std::exception_ptr, Ts...), std::exception_ptr & >
{
    std::optional<std::tuple<Ts...>> &result;
    Channel & channel;
    std::exception_ptr & ec;
    explicit channel_receive(Channel & channel, std::exception_ptr & ec) : channel(channel), ec(ec) {}

    bool await_ready()
    {
        return channel.try_receive(
                [this](std::exception_ptr & ec, auto && ... args)
                {
                    ec = ec;
                    result = {std::move(args)...};
                });
    }

    template<typename Promise>
    void await_suspend( std::coroutine_handle<Promise> h)
    {
        throw_if_cancelled_impl(h.promise());
        using completion = completion_handler<Promise, Ts...>;
        channel.async_receive(asio::redirect_error(completion{h, result}, ec));
    }

    auto await_resume()
    {
        return forward_result(std::move(*result));
    }
};

}

template<typename ... Ts, typename ... Args>
struct receive<asio::experimental::basic_concurrent_channel<Ts...>, Args...> :
        detail::channel_receive<
                asio::experimental::basic_channel<Ts...>, Args...>
{
    using detail::channel_receive<
            asio::experimental::basic_channel<Ts...>, Args...>::channel_receive;
};

template<typename ... Ts, typename ... Args>
struct receive<asio::experimental::basic_channel<Ts...>, Args...> :
        detail::channel_receive<
                asio::experimental::basic_channel<Ts...>, Args...>
{
    using detail::channel_receive<
            asio::experimental::basic_channel<Ts...>, Args...>::channel_receive;
};

template<typename ... Ts, typename ... Args>
struct send<asio::experimental::basic_concurrent_channel<Ts...>, Args...> :
        detail::channel_send<
                asio::experimental::basic_channel<Ts...>, Args...>
{
    using detail::channel_send<
            asio::experimental::basic_channel<Ts...>, Args...>::channel_send;
};

template<typename ... Ts, typename ... Args>
struct send<asio::experimental::basic_channel<Ts...>, Args...> :
        detail::channel_send<
                asio::experimental::basic_channel<Ts...>, Args...>
{
    using detail::channel_send<
            asio::experimental::basic_channel<Ts...>, Args...>::channel_send;
};

};

template<bool Sync = false>
struct enable_operations : enable_async_operation_interpreted
{
    using enable_async_operation_interpreted::await_transform;
};

template<>
struct enable_operations<true>
{
    template<operation Op>
    auto await_transform(Op && op)
    {
        struct result
        {
            Op op;
            constexpr static bool await_ready() { return true; }
            void await_suspend( std::coroutine_handle<void> h) {}
            auto await_resume()
            {
                return op();
            }
        };

        return result{std::forward<Op>(op)};
    }
};

}

#endif //CORO_OPS_HPP
