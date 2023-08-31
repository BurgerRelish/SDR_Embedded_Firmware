#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>

#include <esp_heap_caps.h>

namespace ps {
    
template <typename T>
class allocator : public std::allocator<T> {
public:
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = MallocAllocator<U>;
    };

    MallocAllocator() noexcept = default;

    template <typename U>
    MallocAllocator(const MallocAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        void* ptr = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

        if (ptr == nullptr) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        p -> ~T();
        std::free(p);
    }
};

}
