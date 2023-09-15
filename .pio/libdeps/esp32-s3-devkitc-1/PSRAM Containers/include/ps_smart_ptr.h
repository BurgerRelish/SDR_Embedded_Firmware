#pragma once

#ifndef PS_SMART_PTR_H
#define PS_SMART_PTR_H
#include <memory>
#include <esp_heap_caps.h>
#include "ps_allocator.h"

namespace ps {

template<class T, class ... Args>
std::shared_ptr<T> make_shared(Args&& ... args) {
    typedef typename std::remove_cv<T>::type _Tp_nc;
    return std::allocate_shared<T>(allocator<T>(),
				       std::forward<Args>(args)...);
}

template<class T>
struct is_shared_ptr : std::false_type {};

template<class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

}



#endif