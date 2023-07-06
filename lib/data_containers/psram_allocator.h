
#ifndef PSRAM_ALLOCATOR_H
#define PSRAM_ALLOCATOR_H

#include <esp_heap_caps.h>
#include <stdexcept>
#include <deque>

/**
 * @brief Allocator template that uses only PSRAM memory.
 *
 * This allocator template allocates and deallocates memory using the PSRAM on the ESP32.
 * It complies with the rules for allocators in C++ and can be used with std::containers.
 */
template <typename T>
class PSRAMAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    /**
     * @brief Constructs an allocator object.
     */
    PSRAMAllocator() noexcept {}

    /**
     * @brief Constructs an allocator object with another allocator of the same type.
     *
     * @param other Another allocator object.
     */
    template <typename U>
    PSRAMAllocator(const PSRAMAllocator<U>& other) noexcept {}

    /**
     * @brief Allocates memory for a single object of type T.
     *
     * @param n Size of the allocation (ignored).
     * @return Pointer to the allocated memory.
     */
    [[nodiscard]] T* allocate(std::size_t n) {
        // Allocate memory from PSRAM
        void* ptr = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    /**
     * @brief Deallocates memory for a single object of type T.
     *
     * @param p Pointer to the memory to deallocate.
     * @param n Size of the allocation (ignored).
     */
    void deallocate(T* p, std::size_t /* n */) noexcept {
        // Free memory back to PSRAM
        heap_caps_free(p);
    }

    /**
     * @brief Returns the maximum number of objects that can be allocated.
     *
     * @return Maximum number of objects that can be allocated.
     */
    std::size_t max_size() const noexcept {
        return (size_type)heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / sizeof(T);
    }
};
/**
 * @brief Determines whether two allocators are equal.
 *
 * @tparam T The type of the allocators.
 * @param lhs The first allocator.
 * @param rhs The second allocator.
 * @return True if the allocators are equal, false otherwise.
 */
template <class T, class U>
bool operator==(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) noexcept {
  return true;
}

/**
 * @brief Determines whether two allocators are not equal.
 *
 * @tparam T The type of the allocators.
 * @param lhs The first allocator.
 * @param rhs The second allocator.
 * @return True if the allocators are not equal, false otherwise.
 */
template <class T, class U>
bool operator!=(const PSRAMAllocator<T>& lhs, const PSRAMAllocator<U>& rhs) noexcept {
  return !(lhs == rhs);
}

#endif