#pragma once

#ifndef PSRAM_ALLOCATOR_H
#define PSRAM_ALLOCATOR_H

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

    void* ptr = heap_caps_malloc(__n * sizeof(_Tp), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
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