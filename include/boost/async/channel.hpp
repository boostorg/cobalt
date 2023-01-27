//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_CHANNEL_HPP
#define BOOST_ASYNC_CHANNEL_HPP

#include <boost/async/this_thread.hpp>
#include <boost/async/with.hpp>

#include <boost/intrusive/list.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/post.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/variant2/variant.hpp>

namespace boost::async
{

template<typename T>
struct channel_reader;

template<typename T>
struct channel
{
  channel(std::size_t limit = 0u,
          asio::io_context::executor_type executor = this_thread::get_executor(),
          container::pmr::memory_resource * resource = this_thread::get_default_resource())
            : buffer_(limit, resource), executor_(executor) {}
  channel(channel && ) = default;

  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() {return executor_;}

  ~channel()
  {
    while (!read_queue_.empty())
      read_queue_.front().awaited_from.reset();

    while (!write_queue_.empty())
      write_queue_.front().awaited_from.reset();

  }
  bool is_open() const {return !is_closed_;}
  void close()
  {
    is_closed_ = true;
    while (!read_queue_.empty())
    {
      auto & op = read_queue_.front();
      op.unlink();
      op.cancelled = true;
      op.cancel_slot.clear();
      asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
    }
    while (!write_queue_.empty())
    {
      auto & op = write_queue_.front();
      op.unlink();
      op.cancelled = true;
      op.cancel_slot.clear();
      asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
    }
  }

 private:
  boost::circular_buffer<T, container::pmr::polymorphic_allocator<T>> buffer_;
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false;
    asio::cancellation_slot cancel_slot;

    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};

    struct cancel_impl
    {
      read_op * op;
      cancel_impl(read_op * op) : op(op) {}
      void operator()(asio::cancellation_type ct)
      {
        op->cancelled = true;
        op->unlink();
        asio::post(
            op->chn->executor_,
            [h = std::move(op->awaited_from)]() mutable
            {
              std::coroutine_handle<void>::from_address(h.release()).resume();
            });
      }
    };

    bool await_ready()
    {
      return !chn->buffer_.empty();
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
          (cancel_slot = sl).template emplace<cancel_impl>(this);

      awaited_from.reset(h.address());
      // currently nothing to read
      chn->read_queue_.push_back(*this);

      if (chn->write_queue_.empty())
        return std::noop_coroutine();
      else
      {
        auto & op = chn->write_queue_.front();
        op.unlink();
        BOOST_ASSERT(op.awaited_from);
        return std::coroutine_handle<void>::from_address(op.awaited_from.release());
      }
    }

    T await_resume()
    {
      if (cancel_slot.is_connected())
        cancel_slot.clear();

      if (cancelled)
        boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
      else
      {
        if (!chn->write_queue_.empty())
        {
          auto & op = chn->write_queue_.front();
          op.unlink();
          BOOST_ASSERT(op.awaited_from);
          asio::post(
              chn->executor_,
              [h = std::move(op.awaited_from)]() mutable
              {
                std::coroutine_handle<void>::from_address(h.release()).resume();
              });
        }
        T value = std::move(chn->buffer_.front());
        chn->buffer_.pop_front();
        return value;
      }
    }
    operator bool() const {return chn && chn->is_open();}
  };

  struct write_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    variant2::variant<T*, const T*> ref;
    boost::source_location loc;
    bool cancelled = false;
    asio::cancellation_slot cancel_slot;

    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};

    struct cancel_impl
    {
      write_op * op;
      cancel_impl(write_op * op) : op(op) {}
      void operator()(asio::cancellation_type ct)
      {
        op->cancelled = true;
        op->unlink();
        asio::post(
            op->chn->executor_,
            [h = std::move(op->awaited_from)]() mutable
            {
              std::coroutine_handle<void>::from_address(h.release()).resume();
            });
      }
    };

    bool await_ready()
    {
      return !chn->buffer_.full();
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
          (cancel_slot = sl).template emplace<cancel_impl>(this);

      awaited_from.reset(h.address());
      // currently nothing to read
      chn->write_queue_.push_back(*this);

      if (chn->read_queue_.empty())
        return std::noop_coroutine();
      else
      {
        auto & op = chn->read_queue_.front();
        op.unlink();
        BOOST_ASSERT(op.awaited_from);
        return std::coroutine_handle<void>::from_address(op.awaited_from.release());
      }
    }

    void await_resume()
    {
      if (cancel_slot.is_connected())
        cancel_slot.clear();
      if (cancelled)
        boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
      else
      {
        if (!chn->read_queue_.empty())
        {
          auto & op = chn->read_queue_.front();
          op.unlink();
          BOOST_ASSERT(op.awaited_from);
          asio::post(
              chn->executor_,
              [h = std::move(op.awaited_from)]() mutable
              {
                std::coroutine_handle<void>::from_address(h.release()).resume();
              });
        }
        if (ref.index() == 0)
          chn->buffer_.push_back(std::move(*variant2::get<0>(ref)));
        else
          chn->buffer_.push_back(*variant2::get<1>(ref));
      }
    }
    operator bool() const {return chn && chn->is_open();}
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
  boost::intrusive::list<write_op, intrusive::constant_time_size<false> > write_queue_;
 public:
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }
  write_op write(const T  && value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(const T  &  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(      T &&  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
  write_op write(      T  &  value, const boost::source_location & loc = BOOST_CURRENT_LOCATION)
  {
    return write_op{{}, this, &value, loc};
  }
};

template<>
struct channel<void>
{
  channel(std::size_t limit = 0u,
          asio::io_context::executor_type executor = this_thread::get_executor())
        : limit_(limit), executor_(executor) {}
  channel(channel && ) = default;

  using executor_type = asio::io_context::executor_type;
  executor_type get_executor() {return executor_;}

  ~channel()
  {
    while (!read_queue_.empty())
      read_queue_.front().awaited_from.reset();

    while (!write_queue_.empty())
      write_queue_.front().awaited_from.reset();

  }
  bool is_open() const {return !is_closed_;}
  void close()
  {
    is_closed_ = true;
    while (!read_queue_.empty())
    {
      auto & op = read_queue_.front();
      op.unlink();
      op.cancelled = true;
      op.cancel_slot.clear();
      asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
    }
    while (!write_queue_.empty())
    {
      auto & op = write_queue_.front();
      op.unlink();
      op.cancelled = true;
      op.cancel_slot.clear();
      asio::post(executor_, [&op]{std::coroutine_handle<void>::from_address(op.awaited_from.release()).resume(); });
    }
  }

 private:
  std::size_t limit_;
  std::size_t n_{0u};
  executor_type executor_;
  bool is_closed_{false};

  struct read_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false;
    asio::cancellation_slot cancel_slot;

    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};

    struct cancel_impl
    {
      read_op * op;
      cancel_impl(read_op * op) : op(op) {}
      void operator()(asio::cancellation_type ct)
      {
        op->cancelled = true;
        op->unlink();
        asio::post(
            op->chn->executor_,
            [h = std::move(op->awaited_from)]() mutable
            {
               std::coroutine_handle<void>::from_address(h.release()).resume();
            });
      }
    };

    bool await_ready()
    {
      return (chn->n_ > 0);
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
          (cancel_slot = sl).template emplace<cancel_impl>(this);

      awaited_from.reset(h.address());
      // currently nothing to read
      chn->read_queue_.push_back(*this);

      if (chn->write_queue_.empty())
        return std::noop_coroutine();
      else
      {
        auto & op = chn->write_queue_.front();
        op.unlink();
        BOOST_ASSERT(op.awaited_from);
        return std::coroutine_handle<void>::from_address(op.awaited_from.release());
      }
    }

    void await_resume()
    {
      if (cancel_slot.is_connected())
          cancel_slot.clear();

      if (cancelled)
        boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
      else
      {
        if (!chn->write_queue_.empty())
        {
          auto & op = chn->write_queue_.front();
          op.unlink();
          BOOST_ASSERT(op.awaited_from);
          asio::post(
              chn->executor_,
              [h = std::move(op.awaited_from)]() mutable
              {
                std::coroutine_handle<void>::from_address(h.release()).resume();
              });
        }
        chn->n_--;
      }

    }
  };

  struct write_op : intrusive::list_base_hook<intrusive::link_mode<intrusive::auto_unlink> >
  {
    channel * chn;
    boost::source_location loc;
    bool cancelled = false;
    asio::cancellation_slot cancel_slot;

    std::unique_ptr<void, detail::coro_deleter<>> awaited_from{nullptr};

    struct cancel_impl
    {
      write_op * op;
      cancel_impl(write_op * op) : op(op) {}
      void operator()(asio::cancellation_type ct)
      {
        op->cancelled = true;
        op->unlink();
        asio::post(
            op->chn->executor_,
            [h = std::move(op->awaited_from)]() mutable
            {
              std::coroutine_handle<void>::from_address(h.release()).resume();
            });
      }
    };

    bool await_ready()
    {
      return chn->n_ < chn->limit_;
    }

    template<typename Promise>
    std::coroutine_handle<void> await_suspend(std::coroutine_handle<Promise> h)
    {
      if constexpr (requires (Promise p) {p.get_cancellation_slot();})
        if (auto sl = h.promise().get_cancellation_slot(); sl.is_connected())
          (cancel_slot = sl).template emplace<cancel_impl>(this);

      awaited_from.reset(h.address());
      // currently nothing to read
      chn->write_queue_.push_back(*this);

      if (chn->read_queue_.empty())
        return std::noop_coroutine();
      else
      {
        auto & op = chn->read_queue_.front();
        op.unlink();
        BOOST_ASSERT(op.awaited_from);
        return std::coroutine_handle<void>::from_address(op.awaited_from.release());
      }
    }

    void await_resume()
    {
      if (cancel_slot.is_connected())
        cancel_slot.clear();
      if (cancelled)
        boost::throw_exception(system::system_error(asio::error::operation_aborted), loc);
      else
      {
        if (!chn->read_queue_.empty())
        {
          auto & op = chn->read_queue_.front();
          op.unlink();
          BOOST_ASSERT(op.awaited_from);
          asio::post(
              chn->executor_,
              [h = std::move(op.awaited_from)]() mutable
              {
                std::coroutine_handle<void>::from_address(h.release()).resume();
              });
        }

        chn->n_++;
      }

    }
  };

  boost::intrusive::list<read_op,  intrusive::constant_time_size<false> > read_queue_;
  boost::intrusive::list<write_op, intrusive::constant_time_size<false> > write_queue_;
 public:
  read_op   read(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return  read_op{{}, this, loc}; }
  write_op write(const boost::source_location & loc = BOOST_CURRENT_LOCATION)  {return write_op{{}, this, loc}; }
};

template<typename T>
struct channel_reader
{
  channel_reader(channel<T> & chan,
                 const boost::source_location & loc = BOOST_CURRENT_LOCATION) : chan_(chan), loc_(loc) {}

  auto operator co_await () -> typename channel<T>::read_op
  {
    return chan_->read(loc_);
  }

  explicit operator bool () const {return chan_->is_open();}

 private:
  channel<T> * chan_;
  boost::source_location loc_;
};

}

#endif //BOOST_ASYNC_CHANNEL_HPP
