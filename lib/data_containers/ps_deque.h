

#ifndef PS_DEQUE
#define PS_DEQUE 1

#include <deque>
#include "psram_allocator.h"

template <class T>
using ps_deque = std::deque<T, PSRAMAllocator<T>>;

template <class T>
bool operator==(const ps_deque<T>& lhs, const std::deque<T>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    auto lhsIt = lhs.begin();
    auto rhsIt = rhs.begin();

    while (lhsIt != lhs.end() && rhsIt != rhs.end()) {
        if (*lhsIt != *rhsIt) {
        return false;
        }

        ++lhsIt;
        ++rhsIt;
    }

    return true;
}

template <class T>
bool operator==(const std::deque<T>& lhs, const ps_deque<T>& rhs) {
    return (rhs == lhs);
}

template <class T>
bool operator!=(const std::deque<T>& lhs, const ps_deque<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps_deque<T>& lhs, const std::deque<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
ps_deque<T>& operator<<=(ps_deque<T>& dest, const std::deque<T>& src) {
    dest.clear(); // Clear the destination deque

    // Copy the elements from src to dest
    for (const auto& elem : src) {
        dest.push_back(elem);
    }

    return dest;
}

template <class T>
std::deque<T>& operator<<=(std::deque<T>& dest, const ps_deque<T>& src) {
    dest.clear(); // Clear the destination deque

    // Copy the elements from src to dest
    for (const auto& elem : src) {
        dest.push_back(elem);
    }

    return dest;
}

#endif