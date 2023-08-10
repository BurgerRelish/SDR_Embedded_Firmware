#pragma once

#ifndef PS_SMART_PTR_H
#define PS_SMART_PTR_H
#include <memory>
#include <esp_heap_caps.h>
#include "ps_allocator.h"

namespace ps {

template<class T>
struct Deleter {
    void operator()(T* ptr) const {
        if (ptr) {
            ptr -> ~T();
        }
        
        PS_Allocator<T>().deallocate(ptr, 1);
    }
};


template<class T, class ... Args>
std::shared_ptr<T> make_shared(Args&& ... args) {
    auto memory = PS_Allocator<T>().allocate(sizeof(T));
    if (!memory) {
        throw std::bad_alloc();
    }

    try {
        auto obj = new (memory) T (std::forward<Args>(args)...);

        using AllocatorType = typename std::allocator_traits<PS_Allocator<T>>::template rebind_alloc<T>;

        return std::shared_ptr<T>(obj, Deleter<T>(), AllocatorType(PS_Allocator<T>()));
    } catch (...) {
        PS_Allocator<T>().deallocate(memory, 1);
        throw;
    }
}

template<class T, class ... Args>
std::unique_ptr<T, Deleter<T>> make_unique(Args&& ... args) {
    auto memory = PS_Allocator<T>().allocate(sizeof(T));
    if (!memory) {
        throw std::bad_alloc();
    }

    try {
        auto obj = new (memory) T (std::forward<Args>(args)...);
        return std::unique_ptr<T, Deleter<T>>(obj, Deleter<T>());
    } catch (...) {
        PS_Allocator<T>().deallocate(memory, 1);
        throw;
    }
}

}



#endif