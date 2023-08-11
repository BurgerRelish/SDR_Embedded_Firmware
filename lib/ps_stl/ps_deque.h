

#ifndef PS_DEQUE_H
#define PS_DEQUE_H 1

#include <deque>
#include "ps_allocator.h"

namespace ps {
    template <class T>
    using deque = std::deque<T, allocator<T>>;
}


template <class T>
bool operator==(const ps::deque<T>& lhs, const std::deque<T>& rhs) {
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
bool operator==(const std::deque<T>& lhs, const ps::deque<T>& rhs) {
    return (rhs == lhs);
}

template <class T>
bool operator!=(const std::deque<T>& lhs, const ps::deque<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps::deque<T>& lhs, const std::deque<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
ps::deque<T>& operator<<=(ps::deque<T>& dest, const std::deque<T>& src) {
    dest.clear(); // Clear the destination deque

    // Copy the elements from src to dest
    for (const auto& elem : src) {
        dest.push_back(elem);
    }

    return dest;
}

template <class T>
std::deque<T>& operator<<=(std::deque<T>& dest, const ps::deque<T>& src) {
    dest.clear(); // Clear the destination deque

    // Copy the elements from src to dest
    for (const auto& elem : src) {
        dest.push_back(elem);
    }

    return dest;
}

#endif