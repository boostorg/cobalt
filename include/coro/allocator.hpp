// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CORO_ALLOCATOR_HPP
#define CORO_ALLOCATOR_HPP

#include <asio/associated_allocator.hpp>
#include <coro/util.hpp>

#include <memory_resource>
#include <memory>
#include <limits>

namespace coro
{

namespace this_coro
{

struct allocator_t {};
constexpr allocator_t allocator;

}

namespace this_thread
{


namespace detail
{
  inline static thread_local std::pmr::memory_resource * default_coro_memory_resource = std::pmr::get_default_resource();
}



inline std::pmr::memory_resource* get_default_resource() noexcept
{
    return detail::default_coro_memory_resource;
}

inline std::pmr::memory_resource* set_default_resource(std::pmr::memory_resource* r) noexcept
{
  auto pre = get_default_resource();
  detail::default_coro_memory_resource = r;
  return pre;
}


inline std::pmr::polymorphic_allocator<void> get_allocator()
{
  return std::pmr::polymorphic_allocator<void>(get_default_resource());
}

}

struct promise_memory_resource_base
{
    using allocator_type = std::pmr::polymorphic_allocator<void>;
    allocator_type get_allocator() const {return allocator_type{resource};}

    void * operator new(const std::size_t size)
    {
        auto res = this_thread::get_default_resource();
        const auto p = res->allocate(size + sizeof(std::pmr::memory_resource *), alignof(std::pmr::memory_resource *));
        auto pp = static_cast<std::pmr::memory_resource**>(p);
        *pp = res;
        return pp + 1;
    }

    void operator delete(void * raw, const std::size_t size)
    {
        const auto p = static_cast<std::pmr::memory_resource**>(raw) - 1;
        std::pmr::memory_resource * res = *p;
        res->deallocate(p, size + sizeof(std::pmr::memory_resource *), alignof(std::pmr::memory_resource *));
    }

    promise_memory_resource_base(std::pmr::memory_resource * resource = this_thread::get_default_resource()) : resource(resource) {}

private:
    std::pmr::memory_resource * resource = this_thread::get_default_resource();
};

/// Allocate the memory and put the allocator behind the coro memory
template<typename AllocatorType>
void *allocate_coroutine(const std::size_t size, AllocatorType alloc_)
{
    using alloc_type = typename std::allocator_traits<AllocatorType>::template rebind_alloc<unsigned char>;
    alloc_type alloc{alloc_};

    const auto align_needed = size % alignof(alloc_type);
    const auto align_offset = align_needed != 0 ? alignof(alloc_type) - align_needed : 0ull;
    const auto alloc_size = size + sizeof(alloc_type) + align_offset;
    const auto raw = std::allocator_traits<alloc_type>::allocate(alloc, alloc_size);
    new(raw + size + align_offset) alloc_type(std::move(alloc));

    return raw;
}

/// Deallocate the memory and destroy the allocator in the coro memory.
template<typename AllocatorType>
void deallocate_coroutine(void *raw_, const std::size_t size)
{
    using alloc_type = typename std::allocator_traits<AllocatorType>::template rebind_alloc<unsigned char>;
    const auto raw = static_cast<unsigned char *>(raw_);

    const auto align_needed = size % alignof(alloc_type);
    const auto align_offset = align_needed != 0 ? alignof(alloc_type) - align_needed : 0ull;
    const auto alloc_size = size + sizeof(alloc_type) + align_offset;
    auto alloc_p = reinterpret_cast<alloc_type *>(raw + size + align_offset);
    auto alloc = std::move(*alloc_p);
    alloc_p->~alloc_type();
    std::allocator_traits<alloc_type>::deallocate(alloc, raw, alloc_size);
}


template<typename Promise>
struct enable_await_allocator
{
    auto await_transform(this_coro::allocator_t)
    {
        struct awaitable
        {
            using allocator_type = typename Promise::allocator_type;

            allocator_type alloc;
            constexpr static bool await_ready() { return true; }

            bool await_suspend( std::coroutine_handle<void> h) { return false; }
            allocator_type await_resume()
            {
                return alloc;
            }
        };

        return awaitable{static_cast<Promise*>(this)->get_allocator()};
    }
};


}

#endif //CORO_ALLOCATOR_HPP
