//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASYNC_IO_BUFFERS_IMPL_DYNAMIC_BUFFER_VIEW_HPP
#define BOOST_ASYNC_IO_BUFFERS_IMPL_DYNAMIC_BUFFER_VIEW_HPP

#include <boost/async/io/buffers/dynamic_buffer_view.hpp>

namespace boost::async::io::buffers
{

struct dynamic_buffer_view::vtables
{

  template<typename Buffer>
  constexpr static dynamic_buffer_view::vtable_t dynamic_buffer_vtable_{
      .size     =[](const void *this_) { return static_cast<const Buffer *>(this_)->size(); },
      .max_size =[](const void *this_) { return static_cast<const Buffer *>(this_)->max_size(); },
      .capacity =[](const void *this_) { return static_cast<const Buffer *>(this_)->capacity(); },
      .data     =[](const void *this_, std::size_t)
      {
        auto d = static_cast<const Buffer *>(this_)->data();
        dynamic_buffer_view::const_buffers_type cbt{begin(d), end(d)};
        return cbt;
      },
      .prepare  =[](void *this_, std::size_t n, std::size_t)
      {
        auto p = static_cast<Buffer *>(this_)->prepare(n);
        dynamic_buffer_view::mutable_buffers_type mbt{begin(p), end(p)};
        return mbt;
      },
      .commit   =[](void *this_, std::size_t n, std::size_t &) { static_cast<Buffer *>(this_)->commit(n); },
      .consume  =[](void *this_, std::size_t n, std::size_t &) { static_cast<Buffer *>(this_)->consume(n); }
  };


  template<typename Buffer>
  constexpr static dynamic_buffer_view::vtable_t contiguous_vtable_{
      .size     = [](const void *this_) { return static_cast<const Buffer *>(this_)->size(); },
      .max_size = [](const void *this_) { return static_cast<const Buffer *>(this_)->max_size(); },
      .capacity = [](const void *this_) { return static_cast<const Buffer *>(this_)->capacity(); },
      .data     =[](const void *this_, std::size_t position)
      {
        auto self = static_cast<const Buffer *>(this_);
        const auto p = self->data();
        const_buffer cb{p, (std::min)(position, self->size())};
        dynamic_buffer_view::const_buffers_type cbt{cb};
        return cbt;
      },
      .prepare  =[](void *this_, std::size_t n, std::size_t position)
      {
        auto self = static_cast<Buffer *>(this_);
        auto sz = (std::min)(position + n, self->max_size());
        if (sz > self->size())
          self->resize(sz);

        mutable_buffer buf{*self};
        buf += position;
        return mutable_buffers_type{buf};
      },
      .commit   =[](void *this_, std::size_t n, std::size_t &position)
      {
        position += n;
      },
      .consume  =[](void *this_, std::size_t n, std::size_t &position)
      {
        auto self = static_cast<Buffer *>(this_);
        std::size_t consume_length = (std::min)(n, position);
        self->erase(self->begin(), std::next(self->begin(), consume_length));
        position -= consume_length;
      }
  };


  template<typename Buffer>
  constexpr static dynamic_buffer_view::vtable_t circular_vtable_{
      .size     = [](const void *this_) { return static_cast<const Buffer *>(this_)->size(); },
      .max_size = [](const void *this_) { return static_cast<const Buffer *>(this_)->max_size(); },
      .capacity = [](const void *this_) { return static_cast<const Buffer *>(this_)->capacity(); },
      .data     = [](const void *this_, std::size_t position)
      {
        auto self = static_cast<const Buffer *>(this_);
        const_buffers_type res;

        const_buffer_pair tmp{
            self->array_one(),
            self->array_two()
        };
        auto cb = prefix(tmp, position);

        for (auto val: cb)
          if (val.size() > 0u)
            res.push_back(val);
        return res;
      },
      .prepare = [](void *this_, std::size_t n, std::size_t position)
      {
        auto self = static_cast<Buffer *>(this_);

        auto sz = (std::min)(position + n, self->max_size());
        if (sz > self->size())
          self->resize(sz);

        mutable_buffer_pair buf{self->array_one(), self->array_two()};
        mutable_buffers_type res;
        for (auto val: sans_prefix(buf, position))
          if (val.size() > 0u)
            res.push_back(val);
        return res;
      },
      .commit   =[](void *this_, std::size_t n, std::size_t &position)
      {
        position += n;
      },
      .consume  =[](void *this_, std::size_t n, std::size_t &position)
      {
        auto self = static_cast<Buffer *>(this_);
        self->erase_begin(n);
        position -= n;
      }
  };

  template<typename Buffer>
  constexpr static dynamic_buffer_view::vtable_t deque_vtable_{
      .size     = [](const void *this_) { return static_cast<const Buffer *>(this_)->size(); },
      .max_size = [](const void *this_) { return static_cast<const Buffer *>(this_)->max_size(); },
      .capacity = [](const void *this_) { return static_cast<const Buffer *>(this_)->size(); },
      .data     = [](const void *this_, std::size_t position)
      {
        auto self = static_cast< const Buffer *>(this_);
        position = (std::min)(position, self->size());
        const_buffers_type res;

        const auto bs = self->get_block_size();
        auto itr = self->begin();
        auto fbs = (std::min)(static_cast<std::size_t>(itr.get_last() - itr.get_cur()), position);

        res.emplace_back(itr.get_cur(), fbs);
        position -= fbs;
        itr = std::next(itr, fbs);
        while (position > 0u && itr != self->end() && (res.size() < res.capacity()))
        {
          auto seg = (std::min)(position, bs);
          res.emplace_back(itr.get_cur(), seg);
          position -= seg;
          itr = std::next(itr, seg);
        }
        return res;
      },
      .prepare = [](void *this_, std::size_t n, std::size_t position)
      {
        auto self = static_cast< Buffer *>(this_);
        mutable_buffers_type res;

        auto sz = (std::min)(position + n, self->max_size());
        if (sz > self->size())
          self->resize(sz);

        auto itr = (std::min)(std::next(self->begin(), position), self->end());
        auto seg = itr, end = self->end();

        while (itr != self->end())
        {
          auto ptr = itr++;
          position--;
          if (itr == end)
          {
            res.emplace_back(&*seg, std::distance(seg, itr));
            break;
          }

          if ((&*ptr + 1) == &*itr) // aligned
            continue;

          res.emplace_back(&*seg, std::distance(seg, itr));
          seg = itr;
          if (res.size() == res.capacity())
            break ;
        }
        // segment the thingy.
        return res;
      },
      .commit   =[](void *this_, std::size_t n, std::size_t &position)
      {
        position += n;
      },
      .consume  =[](void *this_, std::size_t n, std::size_t &position)
      {
        auto self = static_cast<Buffer *>(this_);
        using type = typename Buffer::value_type;
        self->erase(self->begin(), (std::min)(std::next(self->begin(), n), self->end()));
        position -= n;
      }
  };

};

template<dynamic_buffer Buffer>
dynamic_buffer_view::dynamic_buffer_view(Buffer &other)
  : this_(&other), vtable_(vtables::dynamic_buffer_vtable_<Buffer>)
{}

template<typename Container >
  requires (
      requires (const Container & ct)
      {
        {ct.data()} -> std::convertible_to<const void*>;
        {ct.size()} -> std::convertible_to<std::size_t>;
        {ct.max_size()} -> std::convertible_to<std::size_t>;
        {ct.capacity()} -> std::convertible_to<std::size_t>;
      }
      &&        requires (Container & ct)
      {
        {ct.data()} -> std::convertible_to<void*>;
        {ct.resize(std::size_t())};
        {ct.erase(ct.begin(), ct.end())};
      }
      && std::is_trivial_v<typename Container::value_type>
      && sizeof(typename Container::value_type) == 1u)
dynamic_buffer_view::dynamic_buffer_view(Container  &other)
    : this_(&other), vtable_(vtables::contiguous_vtable_<Container >)
{
}

template<typename T, typename Allocator>
  requires (std::is_trivial_v<T> && sizeof(T) == 1u)
dynamic_buffer_view::dynamic_buffer_view(circular_buffer<T, Allocator> &other)
    : this_(&other), vtable_(vtables::circular_vtable_<circular_buffer<T, Allocator>>)
{
}

template<typename T, typename Allocator, typename Options>
requires (std::is_trivial_v<T> && sizeof(T) == 1u)
dynamic_buffer_view::dynamic_buffer_view(boost::container::deque<T, Allocator, Options> &other)
    : this_(&other), vtable_(vtables::deque_vtable_<boost::container::deque<T, Allocator, Options>>) {}

}

#endif //BOOST_ASYNC_IO_BUFFERS_IMPL_DYNAMIC_BUFFER_VIEW_HPP
