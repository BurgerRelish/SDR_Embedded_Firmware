#pragma once

#ifndef PSRAM_ALLOCATOR_H
#define PSRAM_ALLOCATOR_H

// #include <esp_heap_caps.h>
// #include <stdexcept>
// #include <deque>

// /**
//  * @brief Allocator template that uses only PSRAM memory.
//  *
//  * This allocator template allocates and deallocates memory using the PSRAM on the ESP32.
//  * It complies with the rules for allocators in C++ and can be used with std::containers.
//  */
// template <typename T>
// class PS_Allocator {
// public:
//     using value_type = T;
//     using pointer = T*;
//     using const_pointer = const T*;
//     using reference = T&;
//     using const_reference = const T&;
//     using difference_type = std::ptrdiff_t;
//     using size_type = std::size_t;

//     template <typename U>
//     struct rebind {
//         using other = PS_Allocator<U>;
//     };

//     /**
//      * @brief Constructs an allocator object.
//      */
//     PS_Allocator() noexcept {}

//     /**
//      * @brief Constructs an allocator object with another allocator of the same type.
//      *
//      * @param other Another allocator object.
//      */
//     template <typename U>
//     PS_Allocator(const PS_Allocator<U>& other) noexcept {}

//     /**
//      * @brief Allocates memory for a single object of type T.
//      *
//      * @param n Size of the allocation (ignored).
//      * @return Pointer to the allocated memory.
//      */
//     [[nodiscard]] T* allocate(std::size_t n) {
//         // Allocate memory from PSRAM
//         void* ptr = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//         if (ptr == nullptr) {
//             throw std::bad_alloc();
//         }
//         return static_cast<T*>(ptr);
//     }

//     /**
//      * @brief Deallocates memory for a single object of type T.
//      *
//      * @param p Pointer to the memory to deallocate.
//      * @param n Size of the allocation (ignored).
//      */
//     void deallocate(T* p, std::size_t /* n */) noexcept {
//         // Free memory back to PSRAM
//         heap_caps_free(p);
//     }

//     /**
//      * @brief Returns the maximum number of objects that can be allocated.
//      *
//      * @return Maximum number of objects that can be allocated.
//      */
//     std::size_t max_size() const noexcept {
//         return (size_type)heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / sizeof(T);
//     }
// };


// /**
//  * @brief Determines whether two allocators are equal.
//  *
//  * @tparam T The type of the allocators.
//  * @param lhs The first allocator.
//  * @param rhs The second allocator.
//  * @return True if the allocators are equal, false otherwise.
//  */
// template <class T, class U>
// bool operator==(const PS_Allocator<T>&, const PS_Allocator<U>&) noexcept {
//   return true;
// }

// /**
//  * @brief Determines whether two allocators are not equal.
//  *
//  * @tparam T The type of the allocators.
//  * @param lhs The first allocator.
//  * @param rhs The second allocator.
//  * @return True if the allocators are not equal, false otherwise.
//  */
// template <class T, class U>
// bool operator!=(const PS_Allocator<T>& lhs, const PS_Allocator<U>& rhs) noexcept {
//   return !(lhs == rhs);
// }

// #include <cstddef>
// #include <cstdlib>
// #include <memory>
// #include <bits/allocator.h>

// #include <esp_heap_caps.h>

// namespace ps {
    
// template <typename T>
// class psram_allocator {
// public:
//     using value_type = T;
//     using pointer = T*;

//     template <typename U>
//     struct rebind {
//         using other = allocator<U>;
//     };

//     allocator() noexcept = default;

//     template <typename U>
//     allocator(const allocator<U>&) noexcept {}

//     [[nodiscard]] T* allocate(std::size_t n) {
//         void* ptr = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

//         if (ptr == nullptr) {
//             throw std::bad_alloc();
//         }

//         return static_cast<T*>(ptr);
//     }

//     void deallocate(T* p, std::size_t n) noexcept {
//         heap_caps_free(p);
//     }
// };

#include <bits/c++config.h>
#include <new>
#include <bits/functexcept.h>
#include <bits/move.h>
#if __cplusplus >= 201103L
#include <type_traits>
#endif

#include "esp_heap_caps.h"

namespace psram {

/**
 * @brief This is a version of the GNU C++ new_allocator, modified by myself.
 * 
 */


 /**
   *  @brief  An allocator that uses PSRAM.
   *  @ingroup allocators
   *    - all allocation calls heap_caps_malloc
   *    - all deallocation calls heap_caps_free
   *
   *  @tparam  _Tp  Type of allocated object.
   */
template<typename _Tp>
class psram_base_allocator
    {
    public:
      typedef size_t     size_type;
      typedef ptrdiff_t  difference_type;
      typedef _Tp*       pointer;
      typedef const _Tp* const_pointer;
      typedef _Tp&       reference;
      typedef const _Tp& const_reference;
      typedef _Tp        value_type;

      template<typename _Tp1>
	struct rebind
	{ typedef psram_base_allocator<_Tp1> other; };

#if __cplusplus >= 201103L
      // _GLIBCXX_RESOLVE_LIB_DEFECTS
      // 2103. propagate_on_container_move_assignment
      typedef std::true_type propagate_on_container_move_assignment;
#endif

      psram_base_allocator() _GLIBCXX_USE_NOEXCEPT { }

      psram_base_allocator(const psram_base_allocator&) _GLIBCXX_USE_NOEXCEPT { }

      template<typename _Tp1>
	psram_base_allocator(const psram_base_allocator<_Tp1>&) _GLIBCXX_USE_NOEXCEPT { }

      ~psram_base_allocator() _GLIBCXX_USE_NOEXCEPT { }

      pointer
      address(reference __x) const _GLIBCXX_NOEXCEPT
      { return std::__addressof(__x); }

      const_pointer
      address(const_reference __x) const _GLIBCXX_NOEXCEPT
      { return std::__addressof(__x); }

      // NB: __n is permitted to be 0.  The C++ standard says nothing
      // about what the return value is when __n == 0.
      pointer
      allocate(size_type __n, const void* = static_cast<const void*>(0))
      {
	if (__n > this->max_size())
	  std::__throw_bad_alloc();

    void* ptr = heap_caps_malloc(__n * sizeof(_Tp), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if (ptr == nullptr)
        throw std::bad_alloc();
    return static_cast<_Tp*>(ptr);
      }

      // __p is not permitted to be a null pointer.
      void
      deallocate(pointer __p, size_type)
      {
        heap_caps_free(__p);
      }

      size_type
      max_size() const _GLIBCXX_USE_NOEXCEPT
      { return size_t(-1) / sizeof(_Tp); }

#if __cplusplus >= 201103L
      template<typename _Up, typename... _Args>
	void
	construct(_Up* __p, _Args&&... __args)
	{ ::new((void *)__p) _Up(std::forward<_Args>(__args)...); }

      template<typename _Up>
	void
	destroy(_Up* __p) { __p->~_Up(); }
    };

  template<typename _Tp>
    inline bool
    operator==(const psram_base_allocator<_Tp>&, const psram_base_allocator<_Tp>&)
    { return true; }

  template<typename _Tp>
    inline bool
    operator!=(const psram_base_allocator<_Tp>&, const psram_base_allocator<_Tp>&)
    { return false; }

#endif
}


#endif