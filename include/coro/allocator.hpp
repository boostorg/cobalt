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
    return detail::default_coro_memory_resource = r;
}


struct promise_memory_resource_base
{
    using allocator_type = std::pmr::polymorphic_allocator<void>;
    allocator_type get_allocator() const {return allocator_type{resource};}

    void * operator new(const std::size_t size)
    {

        auto res = get_default_resource();
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

    promise_memory_resource_base(std::pmr::memory_resource * resource = get_default_resource()) : resource(resource) {}

private:
    std::pmr::memory_resource * resource = get_default_resource();
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



template<typename Allocator>
struct promise_allocator_arg_base
{
    using allocator_type = Allocator;
    allocator_type get_allocator() const {return alloc_;}

    void * operator new(const std::size_t size)
    {
        return allocate_coroutine<allocator_type>(size, Allocator());
    }

    template<typename ... Args>
    void * operator new(const std::size_t size, Args & ... args)
    {
        return allocate_coroutine<allocator_type>(size,
                                  get_variadic<variadic_first<std::allocator_arg_t, std::decay_t<Args>...>() + 1u>(args...));
    }

    void operator delete(void * raw, const std::size_t size)
    {
        deallocate_coroutine<allocator_type>(raw, size);
    }

    template<typename ... Args>
    promise_allocator_arg_base(Args && ... args) : alloc_(
            get_variadic<variadic_first<std::allocator_arg_t, std::decay_t<Args>...>() + 1u>(args...)) {}

  private:
    allocator_type alloc_;
};

template<typename T>
struct promise_allocator_arg_base<std::allocator<T>>
{
    using allocator_type = std::allocator<void>;
    allocator_type get_allocator() const {return {};}

    template<typename ... Args>
    promise_allocator_arg_base(Args && ... ) {}
};


template<typename T>
struct promise_allocator_arg_base<std::pmr::polymorphic_allocator<T>>
{
    using allocator_type = std::pmr::polymorphic_allocator<T>;
    allocator_type get_allocator() const {return alloc_;}


    void * operator new(const std::size_t size)
    {
        return allocate_coroutine<allocator_type>(size, allocator_type(get_default_resource()));
    }


    template<typename ... Args>
    void * operator new(const std::size_t size, Args & ... args)
    {
        return allocate_coroutine<allocator_type>(size,
                                  get_variadic<variadic_first<std::allocator_arg_t, std::decay_t<Args>...>() + 1u>(args...));
    }

    void operator delete(void * raw, const std::size_t size)
    {
        deallocate_coroutine<allocator_type>(raw, size);
    }

    template<typename ... Args>
    promise_allocator_arg_base(Args && ... args) : alloc_(
            get_variadic<variadic_first<std::allocator_arg_t, std::decay_t<Args>...>() + 1u>(args...)) {}

private:
    allocator_type alloc_;
};


namespace detail
{

template<bool, typename ... Args>
struct promise_allocator_arg_type_impl
{
    using type = std::allocator<void>;
};

template<typename ... Args>
struct promise_allocator_arg_type_impl<true, Args...>
{
    using type = std::decay_t<variadic_element_t<variadic_first<std::allocator_arg_t, Args...>() + 1u, Args...>>;
};

}

template<typename ... Args>
using promise_allocator_arg_type = detail::promise_allocator_arg_type_impl<
        (std::is_same_v<std::decay_t<Args>, std::allocator_arg_t> || ...), Args...>;

template<typename Allocator = std::allocator<void>>
struct allocator_resource : std::pmr::memory_resource
{
    static_assert(alignof(typename std::allocator_traits<Allocator>::value_type) == 1);
    Allocator allocator;

    allocator_resource(Allocator allocator) : allocator(allocator) {}

    bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept override
    {
        auto p = reinterpret_cast<const allocator_resource*>(&other);
        return p != nullptr && p->allocator == allocator;
    }
    void do_deallocate( void* p, std::size_t bytes, std::size_t alignment ) override
    {
        std::allocator_traits<Allocator>::deallocate(allocator, p, bytes);
    }
    void* do_allocate( std::size_t bytes, std::size_t alignment ) override
    {
        return std::allocator_traits<Allocator>::allocate(allocator, bytes);
    }
};

}

#endif //CORO_ALLOCATOR_HPP
