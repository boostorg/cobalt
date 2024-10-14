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

#include <boost/asio/deferred.hpp>


namespace boost::cobalt
{


template<typename ... Args>
struct op
{
  virtual void ready(cobalt::handler<Args...>) {};
  virtual void initiate(cobalt::completion_handler<Args...> complete) = 0 ;
  virtual ~op() = default;

  struct awaitable
  {
    op<Args...> &op_;
    std::optional<std::tuple<Args...>> result;
    detail::sbo_resource &resource;

    awaitable(op<Args...> * op_, detail::sbo_resource &resource) : op_(*op_), resource(resource) {}
    awaitable(awaitable && lhs)
        : op_(lhs.op_)
        , result(std::move(lhs.result))
        , resource(lhs.resource)
    {
    }

    bool await_ready()
    {
      op_.ready(handler<Args...>(result));
      return result.has_value();
    }


    detail::completed_immediately_t completed_immediately = detail::completed_immediately_t::no;
    std::exception_ptr init_ep;

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
        op_.initiate(completion_handler<Args...>{h, result, &resource, &completed_immediately, loc});
#else
        op_.initiate(completion_handler<Args...>{h, result, &resource, &completed_immediately});
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

    auto await_resume(const boost::source_location & loc = BOOST_CURRENT_LOCATION)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return await_resume(as_result_tag{}).value(loc);
    }

    auto await_resume(const struct as_tuple_tag &)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return *std::move(result);
    }

    auto await_resume(const struct as_result_tag &)
    {
      if (init_ep)
        std::rethrow_exception(init_ep);
      return interpret_as_result(*std::move(result));
    }
  };

  awaitable operator co_await() &&
  {
    return awaitable{this, resource_};
  }

 protected:
  char buffer_[BOOST_COBALT_SBO_BUFFER_SIZE];
  detail::sbo_resource resource_{buffer_, sizeof(buffer_)};

};

// a simple op using a function pointer and
template<typename ... Args>
struct any_op final : op<Args...>
{
    void initiate(completion_handler<Args...> handler) final
    {
      initiate_(args_, std::move(handler));
    }

    template<std::invocable<completion_handler<Args...>> Op>
    any_op(Op && op)
    {
      using op_t = std::decay_t<Op>;
      initiate_ = +[](void * op_, completion_handler<Args...> handler) {(*static_cast<op_t*>(op_))(std::move(handler)); };
      delete_ =   +[](void * op_, detail::sbo_resource & res)
                  {
                    auto * op = static_cast<op_t*>(op_);
                    op->~op_t();
                    res.deallocate(op_, sizeof(op_t), alignof(op_t));
                  };

      move_ = +[](void * op_, detail::sbo_resource & res) -> void*
              {
                auto * op = static_cast<op_t*>(op_);
                auto p = res.allocate(sizeof(op_t), alignof(op_t));

                BOOST_TRY
                {
                  return new (p) op_t(std::move(*op));
                }
                BOOST_CATCH(...)
                {
                  res.deallocate(p, sizeof(op_t), alignof(op_t));
                  throw;
                }
                BOOST_CATCH_END

              };


      auto p = this->resource_.allocate(sizeof(op_t), alignof(op_t));

      BOOST_TRY
      {
        args_ = new (p) op_t(std::forward<Op>(op));
      }
      BOOST_CATCH(...)
      {
        this->resource_.deallocate(p, sizeof(op_t), alignof(op_t));
        throw;
      }
      BOOST_CATCH_END

    }
    any_op() = delete;
    any_op(const any_op &) = delete;
    ~any_op()
    {
      delete_(args_, this->resource_);
    }

    any_op(any_op && lhs)
        : args_(lhs.move_(lhs.args_, this->resource_)),
          initiate_(lhs.initiate_), delete_(lhs.delete_), move_(lhs.move_)
    {
    }
 private:
    void * args_;
    void (*initiate_)(void*, completion_handler<Args...> handler);
    void (*delete_)  (void*, detail::sbo_resource & res);
    void* (*move_)   (void*, detail::sbo_resource & new_res);
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
  using return_type = boost::cobalt::any_op<Args...>;

  template <typename Initiation, typename... InitArgs>
  static auto initiate(Initiation && initiation,
                       boost::cobalt::use_op_t,
                       InitArgs &&... args)
      -> boost::cobalt::any_op<Args...>
  {
    return [initiation = static_cast<Initiation&&>(initiation),
            ...args    = static_cast<InitArgs&&>(args)](cobalt::completion_handler<Args...> handler) mutable
           {
             auto init = static_cast<Initiation&&>(initiation);
             std::move(init)(std::move(handler), static_cast<InitArgs&&>(args)...);
           };
  }
};



}
#endif //BOOST_COBALT_OP_HPP
