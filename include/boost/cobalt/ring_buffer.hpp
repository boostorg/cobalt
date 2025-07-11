//
// Copyright (c) 2025 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_RING_BUFFER_HPP
#define BOOST_COBALT_RING_BUFFER_HPP

#include <boost/cobalt/this_thread.hpp>
#include <boost/cobalt/unique_handle.hpp>
#include <boost/cobalt/detail/util.hpp>

#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/intrusive/list.hpp>

#include <optional>

namespace boost::cobalt
{

// tag::outline[]
template<typename T>
struct ring_buffer
{
  // end::outline[]
#if defined(BOOST_COBALT_NO_PMR)
  explicit
  ring_buffer(std::size_t limit = 0u,
          executor executor = this_thread::get_executor());
#else
  // tag::outline[]
  // create a ring_buffer with a buffer limit, executor & resource.
  explicit
  ring_buffer(std::size_t limit = 0u,
      executor executor = this_thread::get_executor(),
      pmr::memory_resource * resource = this_thread::get_default_resource());
  // end::outline[]
#endif
  // tag::outline[]
  // not movable.
  ring_buffer(ring_buffer && rhs) noexcept = delete;
  ring_buffer & operator=(ring_buffer && lhs) noexcept = delete;

  using executor_type = executor;
  const executor_type & get_executor();

  // Closes the ring_buffer
  ~ring_buffer();
  bool is_open() const;
  // close the operation, will cancel all pending ops, too
  void close();
  // Check how many values are available
  std::size_t available() const;

  // end::outline[]
 private:
#if !defined(BOOST_COBALT_NO_PMR)
  boost::circular_buffer<T, pmr::polymorphic_allocator<T>> buffer_;
#else
  boost::circular_buffer<T> buffer_;
#endif
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    ring_buffer * buf;
    boost::source_location loc;
    bool cancelled = false;
    std::optional<T> direct;
    asio::cancellation_slot cancel_slot{};
    unique_handle<void> awaited_from{nullptr};

    void (*begin_transaction)(void*) = nullptr;

    void transactional_unlink()
    {
      if (begin_transaction)
        begin_transaction(awaited_from.get());
      this->unlink();
    }

    void interrupt_await()
    {
      this->cancelled = true;
      if (this->awaited_from)
          this->awaited_from.release().resume();
    }

    struct cancel_impl;
    bool await_ready() const { return !buf->buffer_.empty() || buf->is_closed_; }
    template<typename Promise>
    BOOST_COBALT_MSVC_NOINLINE
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    T await_resume();
    std::tuple<system::error_code, T> await_resume(const struct as_tuple_tag & );
    system::result<T> await_resume(const struct as_result_tag &);
    explicit operator bool() const {return buf && buf->is_open();}
  };

  struct write_op
  {
    ring_buffer * buf;
    T value;
    boost::source_location loc;
    bool cancelled = false, direct = false;


    void interrupt_await() // if interrupted between ready & suspend
    {
      this->cancelled = true;
    }

    bool await_ready() const { return buf->read_queue_.empty() || buf->is_closed_; }
    template<typename Promise>
    BOOST_COBALT_MSVC_NOINLINE
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);

    void await_resume();
    std::tuple<system::error_code> await_resume(const struct as_tuple_tag & );
    system::result<void> await_resume(const struct as_result_tag &);
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
 public:

  BOOST_COBALT_MSVC_NOINLINE
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }

  BOOST_COBALT_MSVC_NOINLINE
  write_op write(T && value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  write_op{this, std::move(value), loc}; }

  BOOST_COBALT_MSVC_NOINLINE
  write_op write(const T & value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  write_op{this, value, loc}; }


  void post(T && value);

  /*
  // tag::outline[]
  // an awaitable that yields T
  using __read_op__ = __unspecified__;

  // write a value to the ring_buffer
  __write_op__ write(      T &&  value);

  // write a value to the channel if T is void
  __write_op__ write();

  // Post a value to the ring_buffer
  void post(T && value);

  // Post a value if the ring_buffer is void. That is, never block.
  void post();

  // end::outline[]
  */
  // tag::outline[]

};
// end::outline[]


template<>
struct ring_buffer<void>
{
  explicit
  ring_buffer(std::size_t limit = 0u,
              executor executor = this_thread::get_executor())
              : limit_(limit), executor_(executor) {}

  ring_buffer(ring_buffer && rhs) noexcept = delete;
  ring_buffer & operator=(ring_buffer && lhs) noexcept = delete;

  using executor_type = executor;
  const executor_type & get_executor() { return executor_;}

  ~ring_buffer();
  bool is_open() const;
  void close();
  std::size_t available() const noexcept { return n_;}
 private:
  std::size_t limit_;
  std::size_t n_{0u};
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    ring_buffer * buf;
    boost::source_location loc;
    bool cancelled = false, direct=true;
    asio::cancellation_slot cancel_slot{};
    unique_handle<void> awaited_from{nullptr};

    void (*begin_transaction)(void*) = nullptr;

    void transactional_unlink()
    {
      if (begin_transaction)
        begin_transaction(awaited_from.get());
      this->unlink();
    }

    void interrupt_await()
    {
      this->cancelled = true;
      if (this->awaited_from)
        this->awaited_from.release().resume();
    }

    struct cancel_impl;
    bool await_ready() const { return buf->n_ > 0 || buf->is_closed_; }
    template<typename Promise>
    BOOST_COBALT_MSVC_NOINLINE
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);
    BOOST_COBALT_DECL void await_resume();
    BOOST_COBALT_DECL std::tuple<system::error_code> await_resume(const struct as_tuple_tag & );
    BOOST_COBALT_DECL system::result<void> await_resume(const struct as_result_tag &);
    explicit operator bool() const {return buf && buf->is_open();}
  };

  struct write_op
  {
    ring_buffer * buf;
    boost::source_location loc;
    bool cancelled = false, direct = false;


    void interrupt_await() // if interrupted between ready & suspend
    {
      this->cancelled = true;
    }

    bool await_ready() const { return buf->read_queue_.empty() || buf->is_closed_; }

    template<typename Promise>
    BOOST_COBALT_MSVC_NOINLINE
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h);

    void await_resume();
    std::tuple<system::error_code> await_resume(const struct as_tuple_tag & );
    system::result<void> await_resume(const struct as_result_tag &);
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
 public:

  BOOST_COBALT_MSVC_NOINLINE
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }

  BOOST_COBALT_MSVC_NOINLINE
  write_op write(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  write_op{this, loc}; }

  void post();

};

}

#include <boost/cobalt/impl/ring_buffer.hpp>


#endif //BOOST_COBALT_RING_BUFFER_HPP
