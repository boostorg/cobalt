//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_HANDLE_HPP
#define BOOST_ASYNC_HANDLE_HPP

#include <boost/asio/associator.hpp>
#include <coroutine>
#include <memory>

namespace boost::async
{

template<typename T>
struct unique_handle
{
  unique_handle() noexcept = default;
  explicit unique_handle(T * promise) noexcept : handle_(promise) {}
  unique_handle(std::nullptr_t) noexcept {}

  std::coroutine_handle<T> release()
  {
    return std::coroutine_handle<T>::from_promise(*handle_.release());
  }

  void* address() const noexcept { return get_handle_().address(); }
  static unique_handle from_address(void* a) noexcept
  {
    unique_handle res;
    res.handle_.reset(&std::coroutine_handle<T>::from_address(a).promise());
    return res;
  }

  bool done() const noexcept { return get_handle_().done(); }
  explicit operator bool() const { return static_cast<bool>(handle_); }

  void destroy() { handle_.reset(); }

  void operator()() const & { if(*this) resume(); }
  void resume()     const & { get_handle_().resume(); }

  void operator()() && { if(*this) release().resume(); }
  void resume()     && { release().resume(); }

  T & promise() {return *handle_;}
  constexpr static unique_handle from_promise(T &p) noexcept
  {
    unique_handle res;
    res.handle_.reset(&p);
    return res;
  }

        T & operator*()       {return *handle_;}
  const T & operator*() const {return *handle_;}


        T * operator->()       {return handle_.get();}
  const T * operator->() const {return handle_.get();}

        T * get()        {return handle_.get();}
  const T * get() const  {return handle_.get();}
  void reset(T * handle = nullptr) {handle_.reset(handle);}

  friend
  auto operator==(const unique_handle & h, std::nullptr_t) {return h.handle_ == nullptr;}
  friend
  auto operator!=(const unique_handle & h, std::nullptr_t) {return h.handle_ != nullptr;}

 private:
  struct deleter_
  {
    void operator()(T * p)
    {
      std::coroutine_handle<T>::from_promise(*p).destroy();
    }
  };

  std::coroutine_handle<T> get_handle_() const
  {
    return std::coroutine_handle<T>::from_promise(*handle_);
  }

  std::unique_ptr<T, deleter_> handle_;
};

template<>
struct unique_handle<void>
{
  unique_handle() noexcept = default;
  unique_handle(std::nullptr_t) noexcept {}
  explicit unique_handle(void * handle)  noexcept : handle_(handle) {}

  std::coroutine_handle<void> release()
  {
    return std::coroutine_handle<void>::from_address(handle_.release());
  }
  void* address() const noexcept { return get_handle_().address(); }
  static unique_handle<void> from_address(void* a) noexcept
  {

    unique_handle res;
    res.handle_.reset(std::coroutine_handle<void>::from_address(a).address());
    return res;
  }

  explicit operator bool() const { return static_cast<bool>(handle_); }
  bool done() const noexcept { return get_handle_().done(); }

  void operator()() const & { if(*this) resume(); }
  void resume() const & { get_handle_().resume(); }

  void operator()() && { if(*this) release().resume(); }
  void resume()     && { release().resume(); }

  void destroy() { handle_.reset(); }

        void * get()       { return handle_.get(); }
  const void * get() const { return handle_.get(); }
  void reset(void * handle = nullptr) {handle_.reset(handle);}

  friend
  auto operator==(const unique_handle & h, std::nullptr_t) {return h.handle_ == nullptr;}
  friend
  auto operator!=(const unique_handle & h, std::nullptr_t) {return h.handle_ != nullptr;}
 private:
  struct deleter_
  {
    void operator()(void * p)
    {
      std::coroutine_handle<void>::from_address(p).destroy();
    }
  };

  std::coroutine_handle<void> get_handle_() const
  {
    return std::coroutine_handle<void>::from_address(handle_.get());
  }

  std::unique_ptr<void, deleter_> handle_;
};

template<>
struct unique_handle<std::noop_coroutine_promise>
{
  unique_handle() noexcept = default;
  unique_handle(std::nullptr_t) noexcept {}

  std::coroutine_handle<void> release()
  {
    return std::noop_coroutine();
  }
  void* address() const noexcept { return std::noop_coroutine().address(); }
  bool done() const noexcept { return true;}
  void operator()() const {}
  void resume() const {}
  void destroy() {}
  explicit operator bool() const { return true; }

  struct executor_type
  {
    template<typename Fn>
    void execute(Fn &&) const {}
  };

  executor_type get_executor() const {return {}; }

  friend
  auto operator==(const unique_handle &, std::nullptr_t) {return false;}
  friend
  auto operator!=(const unique_handle &, std::nullptr_t) {return true;}
};

}

namespace boost::asio
{

template <template <typename, typename> class Associator,
    typename Promise, typename DefaultCandidate>
    requires (!std::is_void_v<Promise>)
struct associator<Associator,
    boost::async::unique_handle<Promise>, DefaultCandidate>
  : Associator<Promise, DefaultCandidate>
{
  static typename Associator<Promise, DefaultCandidate>::type
  get(const boost::async::unique_handle<Promise>& h) BOOST_ASIO_NOEXCEPT
  {
    if (!h)
      throw_exception(system::system_error(error::invalid_argument, "unique_handle-associator"));
    return Associator<Promise, DefaultCandidate>::get(*h);
  }

  static BOOST_ASIO_AUTO_RETURN_TYPE_PREFIX2(
      typename Associator<Handler, DefaultCandidate>::type)
  get(const boost::async::unique_handle<Promise>& h,
      const DefaultCandidate& c) BOOST_ASIO_NOEXCEPT
    BOOST_ASIO_AUTO_RETURN_TYPE_SUFFIX((
      Associator<Promise, DefaultCandidate>::get(*h, c)))
  {
    if (!h)
      throw_exception(
          system::system_error(error::invalid_argument, "unique_handle-associator"));
    return Associator<Promise, DefaultCandidate>::get(*h, c);
  }
};

}


#endif //BOOST_ASYNC_HANDLE_HPP
