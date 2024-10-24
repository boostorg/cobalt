//
// Copyright (c) 2024 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP
#define BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP

#include <boost/asio/buffer.hpp>
#include <boost/asio/registered_buffer.hpp>
#include <span>

namespace boost::cobalt::experimental::io
{

using boost::asio::buffer;

using boost::asio::mutable_buffer;

struct mutable_buffer_sequence
{
  union {
    boost::asio::mutable_buffer head;
    boost::asio::mutable_registered_buffer registered{};
  };
  std::span<const boost::asio::mutable_buffer> tail;

  std::size_t buffer_count() const {return tail.size() + 1u;}

  mutable_buffer_sequence(boost::asio::mutable_registered_buffer buffer = {}) : registered(buffer)
  {
  }

  mutable_buffer_sequence(boost::asio::mutable_buffer head)
  {
    this->head = head;
  }
  mutable_buffer_sequence(std::span<const boost::asio::mutable_buffer> spn)
  {
    if (!spn.empty())
    {
      head = spn.front();
      tail = spn.subspan(1u);
    }
  }

  mutable_buffer_sequence & operator+=(std::size_t n)
  {
    if (n < head.size())
      head += n;
    else
    {
      n -= head.size();
      std::size_t idx = 0u;
      while (idx < tail.size() && n > tail[idx].size() )
        n -= tail[idx++].size();

      if (idx == tail.size()) // we're exceeding the size, so just drop everything
      {
        tail = {};
        head = {};
      }
      if (tail.empty())
        head = {};
      else
      {
        head = tail[idx];
        head += n;
        tail = tail.subspan(idx + 1);
      }
    }
    return *this;
  }

  struct const_iterator
  {
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = boost::asio::mutable_buffer;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const boost::asio::mutable_buffer*;
    using reference         = const boost::asio::mutable_buffer&;

    const_iterator(boost::asio::mutable_buffer head,
                   std::span<const boost::asio::mutable_buffer> tail,
                   std::size_t offset = std::numeric_limits<std::size_t>::max()) : head_(head), tail_(tail), offset_(offset) {}

    reference & operator*() const
    {
      return offset_ == std::numeric_limits<std::size_t>::max() ? head_ : tail_[offset_];
    }

    pointer operator->() const
    {
      return offset_ == std::numeric_limits<std::size_t>::max() ? &head_ : &tail_[offset_];
    }

    const_iterator operator++()
    {
      offset_++;
      return *this;
    }

    const_iterator operator++(int)
    {
      auto o = *this;
      offset_ ++ ;
      return o;
    }

    const_iterator operator--()
    {
      offset_ --;
      return *this;
    }

    const_iterator operator--(int)
    {
      auto o = *this;
      offset_ -- ;
      return o;
    }

    const_iterator operator+(difference_type diff) const
    {
      auto res = *this;
      res.offset_ += diff;
      return res;
    }

    const_iterator operator-(difference_type diff) const
    {
      auto res = *this;
      res.offset_ -= diff;
      return res;
    }

    const_iterator& operator+=(difference_type diff)
    {
      offset_ += diff;
      return *this;
    }

    const_iterator operator-=(difference_type diff)
    {
      offset_ -= diff;
      return *this;
    }

    reference operator[](difference_type n) const
    {
      auto idx = offset_ + n;
      return idx == std::numeric_limits<std::size_t>::max() ? head_ : tail_[idx];
    }

    difference_type operator-(const_iterator itr) const
    {
      return static_cast<difference_type>(offset_) - static_cast<difference_type>(itr.offset_);
    }

    friend auto operator<=>(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
             <=> std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    };

    friend bool operator==(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
          != std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    }

    friend bool operator!=(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
             != std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    }
   private:

    boost::asio::mutable_buffer head_;
    std::span<const boost::asio::mutable_buffer> tail_;

    std::size_t offset_{std::numeric_limits<std::size_t>::max()};
  };

  const_iterator begin() const {return const_iterator{head, tail};}
  const_iterator   end() const {return const_iterator{head, tail, tail.size()};}

  bool is_registered() const
  {
    return registered.id() != boost::asio::registered_buffer_id();
  }
};


using boost::asio::const_buffer;

struct const_buffer_sequence
{
  union {
    boost::asio::const_buffer head;
    boost::asio::const_registered_buffer registered;
  };
  std::span<const boost::asio::const_buffer> tail;

  std::size_t buffer_count() const {return tail.size() + 1u;}

  const_buffer_sequence(boost::asio::const_registered_buffer buffer = {}) : registered(buffer)
  {
  }

  const_buffer_sequence(boost::asio::const_buffer head)  { this->head = head; }
  const_buffer_sequence(std::span<const boost::asio::const_buffer> spn)
  {
    if (!spn.empty())
    {
      head = spn.front();
      tail = spn.subspan(1u);
    }
  }

  const_buffer_sequence& operator=(boost::asio::const_buffer cb)
  {
    head = cb;
    tail = {};
    return *this;
  }

  const_buffer_sequence& operator=(std::span<const boost::asio::const_buffer> spn)
  {
    if (!spn.empty())
    {
      head = spn.front();
      tail = spn.subspan(1u);
    }
    else
    {
      head = {};
      tail = {};
    }
    return *this;
  }

  const_buffer_sequence & operator+=(std::size_t n)
  {
    if (n < head.size())
      head += n;
    else
    {
      n -= head.size();
      std::size_t idx = 0u;
      while (idx < tail.size() && n > tail[idx].size() )
        n -= tail[idx++].size();

      if (idx == tail.size()) // we're exceeding the size, so just drop everything
      {
        tail = {};
        head = {};
      }
      if (tail.empty())
        head = {};
      else
      {
        head = tail[idx];
        head += n;
        tail = tail.subspan(idx + 1);
      }
    }
    return *this;
  }

  struct const_iterator
  {
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = boost::asio::const_buffer;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const boost::asio::const_buffer*;
    using reference         = const boost::asio::const_buffer&;

    const_iterator(boost::asio::const_buffer head,
                   std::span<const boost::asio::const_buffer> tail,
                   std::size_t offset = std::numeric_limits<std::size_t>::max()) : head_(head), tail_(tail), offset_(offset) {}

    reference & operator*() const
    {
      return offset_ == std::numeric_limits<std::size_t>::max() ? head_ : tail_[offset_];
    }

    pointer operator->() const
    {
      return offset_ == std::numeric_limits<std::size_t>::max() ? &head_ : &tail_[offset_];
    }

    const_iterator operator++()
    {
      offset_++;
      return *this;
    }

    const_iterator operator++(int)
    {
      auto o = *this;
      offset_ ++ ;
      return o;
    }

    const_iterator operator--()
    {
      offset_ --;
      return *this;
    }

    const_iterator operator--(int)
    {
      auto o = *this;
      offset_ -- ;
      return o;
    }

    const_iterator operator+(difference_type diff) const
    {
      auto res = *this;
      res.offset_ += diff;
      return res;
    }

    const_iterator operator-(difference_type diff) const
    {
      auto res = *this;
      res.offset_ -= diff;
      return res;
    }

    const_iterator& operator+=(difference_type diff)
    {
      offset_ += diff;
      return *this;
    }

    const_iterator operator-=(difference_type diff)
    {
      offset_ -= diff;
      return *this;
    }

    reference operator[](difference_type n) const
    {
      auto idx = offset_ + n;
      return idx == std::numeric_limits<std::size_t>::max() ? head_ : tail_[idx];
    }

    difference_type operator-(const_iterator itr) const
    {
      return static_cast<difference_type>(offset_) - static_cast<difference_type>(itr.offset_);
    }

    friend auto operator<=>(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
             <=> std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    };
    friend bool operator==(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
          != std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    }
    friend bool operator!=(const const_iterator & lhs, const const_iterator & rhs)
    {
      return std::make_tuple(lhs.head_.data(), lhs.head_.size(), lhs.tail_.data(), lhs.tail_.size(), lhs.offset_)
             != std::make_tuple(rhs.head_.data(), rhs.head_.size(), rhs.tail_.data(), rhs.tail_.size(), rhs.offset_);
    }

   private:

    boost::asio::const_buffer head_;
    std::span<const boost::asio::const_buffer> tail_;

    std::size_t offset_{std::numeric_limits<std::size_t>::max()};
  };

  const_iterator begin() const {return const_iterator{head, tail};}
  const_iterator   end() const {return const_iterator{head, tail, tail.size()};}

  bool is_registered() const
  {
    return registered.id() != boost::asio::registered_buffer_id();
  }
};

}

#endif //BOOST_COBALT_EXPERIMENTAL_BUFFER_HPP
