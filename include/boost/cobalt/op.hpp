//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_OP_HPP
#define BOOST_COBALT_OP_HPP

#include <boost/cobalt/detail/handler.hpp>
#include <boost/cobalt/detail/sbo_resource.hpp>
#include <boost/cobalt/result.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/config.hpp>
#include <boost/asio/deferred.hpp>


namespace boost::cobalt
{


template<typename ... Args>
struct BOOST_SYMBOL_VISIBLE op
{
  virtual void ready(cobalt::handler<Args...>) {};
  virtual void initiate(cobalt::completion_handler<Args...> complete) = 0 ;
  virtual ~op() = default;

  struct awaitable_base
  {
    op<Args...> &op_;
    std::optional<std::tuple<Args...>> result;

#if !defined(BOOST_COBALT_NO_PMR)
    using resource_type = pmr::memory_resource;
#else
    using resource_type = detail::sbo_resource;
#endif

    awaitable_base(op<Args...> * op_, resource_type *resource) : op_(*op_), resource(resource) {}
    awaitable_base(awaitable_base && lhs) noexcept = default;

#if defined(_MSC_VER)
    BOOST_NOINLINE ~awaitable_base() {}
#endif

    bool await_ready()
    {
      op_.ready(handler<Args...>(result));
      return result.has_value();
    }

    detail::completed_immediately_t completed_immediately = detail::completed_immediately_t::no;
    std::exception_ptr init_ep;

    resource_type *resource;

    template<typename Promise>
    bool await_suspend(std::coroutine_handle<Promise> h
#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
                     , const boost::source_location & loc = BOOST_CURRENT_LOCATION
#endif
    ) noexcept
    {
      BOOST_TRY
      {
        completed_immediately = detail::completed_immediately_t::initiating;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
        op_.initiate(completion_handler<Args...>{h, result, resource, &completed_immediately, loc});
#else
        op_.initiate(completion_handler<Args...>{h, result, resource, &completed_immediately});
#endif
        if (completed_immediately == detail::completed_immediately_t::initiating)
          completed_immediately = detail::completed_immediately_t::no;
        return completed_immediately != detail::completed_immediately_t::yes;
      }
      BOOST_CATCH(...)
      {
        init_ep = std::current_exception();
        return false;
      }
      BOOST_CATCH_END
    }
    BOOST_COBALT_MSVC_NOINLINE
    auto await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return await_resume(as_result_tag{}).value(loc);
    }

#if defined(_MSC_VER)
    BOOST_NOINLINE
#endif
    auto await_resume(const struct as_tuple_tag &)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return *std::move(result);
    }

#if defined(_MSC_VER)
    BOOST_NOINLINE
#endif
    auto await_resume(const struct as_result_tag &)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return interpret_as_result(*std::move(result));
    }
  };

  struct awaitable : awaitable_base
  {
    char buffer[BOOST_COBALT_SBO_BUFFER_SIZE];
    detail::sbo_resource resource{buffer, sizeof(buffer)};

    awaitable(op<Args...> * op_) : awaitable_base(op_, &resource) {}
    awaitable(awaitable && rhs) : awaitable_base(std::move(rhs))
    {
      this->awaitable_base::resource = &resource;
    }

    awaitable_base replace_resource(typename awaitable_base::resource_type * resource) &&
    {
      awaitable_base nw = std::move(*this);
      nw.resource = resource;
      return nw;
    }
  };

  awaitable operator co_await()
  {
    return awaitable{this};
  }
};

struct use_op_t
{
  /// Default constructor.
  constexpr use_op_t()
  {
  }

  /// Adapts an executor to add the @c use_op_t completion token as the
  /// default.
  template <typename InnerExecutor>
  struct executor_with_default : InnerExecutor
  {
    /// Specify @c use_op_t as the default completion token type.
    typedef use_op_t default_completion_token_type;

    executor_with_default(const InnerExecutor& ex) noexcept
        : InnerExecutor(ex)
    {
    }

    /// Construct the adapted executor from the inner executor type.
    template <typename InnerExecutor1>
    executor_with_default(const InnerExecutor1& ex,
                          typename std::enable_if<
                              std::conditional<
                              !std::is_same<InnerExecutor1, executor_with_default>::value,
                                  std::is_convertible<InnerExecutor1, InnerExecutor>,
                          std::false_type
          >::type::value>::type = 0) noexcept
      : InnerExecutor(ex)
    {
    }
  };

  /// Type alias to adapt an I/O object to use @c use_op_t as its
  /// default completion token type.
  template <typename T>
  using as_default_on_t = typename T::template rebind_executor<
        executor_with_default<typename T::executor_type> >::other;

  /// Function helper to adapt an I/O object to use @c use_op_t as its
  /// default completion token type.
  template <typename T>
  static typename std::decay_t<T>::template rebind_executor<
      executor_with_default<typename std::decay_t<T>::executor_type>
    >::other
  as_default_on(T && object)
  {
    return typename std::decay_t<T>::template rebind_executor<
        executor_with_default<typename std::decay_t<T>::executor_type>
      >::other(std::forward<T>(object));
  }

};

constexpr use_op_t use_op{};

struct enable_await_deferred
{
  template<typename ... Args, typename Initiation, typename ... InitArgs>
  auto await_transform(asio::deferred_async_operation<void(Args...), Initiation, InitArgs...> op_)
  {
    struct deferred_op : op<Args...>
    {
      asio::deferred_async_operation<void(Args...), Initiation, InitArgs...> op_;
      deferred_op(asio::deferred_async_operation<void(Args...), Initiation, InitArgs...> op_)
          : op_(std::move(op_)) {}

      void initiate(cobalt::completion_handler<Args...> complete) override
      {
        std::move(op_)(std::move(complete));
      }
    };

    return deferred_op{std::move(op_)};
  }
};

}

namespace boost::asio
{

template<typename ... Args>
struct async_result<boost::cobalt::use_op_t, void(Args...)>
{
  using return_type = boost::cobalt::op<Args...>;

  template <typename Initiation, typename... InitArgs>
  struct op_impl final : boost::cobalt::op<Args...>
  {
    Initiation initiation;
    std::tuple<InitArgs...> args;
    template<typename Initiation_, typename ...InitArgs_>
    op_impl(Initiation_ initiation,
            InitArgs_   && ... args)
            : initiation(std::forward<Initiation_>(initiation))
            , args(std::forward<InitArgs_>(args)...) {}

    void initiate(cobalt::completion_handler<Args...> complete) final override
    {
      std::apply(
          [&](InitArgs && ... args)
          {
            std::move(initiation)(std::move(complete),
                                  std::move(args)...);
          }, std::move(args));
    }
  };

  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation && initiation,
                       boost::cobalt::use_op_t,
                       InitArgs &&... args)
      -> op_impl<std::decay_t<Initiation>, std::decay_t<InitArgs>...>
  {
    return op_impl<std::decay_t<Initiation>, std::decay_t<InitArgs>...>(
        std::forward<Initiation>(initiation),
        std::forward<InitArgs>(args)...);
  }
};



}
#endif //BOOST_COBALT_OP_HPP
