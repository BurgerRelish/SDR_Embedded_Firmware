
#ifndef PS_VECTOR_H
#define PS_VECTOR_H

#include <vector>
#include "ps_allocator.h"

namespace ps {
    /**
     * @brief An implementation of the std::vector but using the ESP32's PSRAM.
     * @note The operator '<<=' can be used to COPY a ps::vector to a std::vector or vice versa.
     * @note The operator '>>=' can be used to MOVE a ps::vector to a std::vector or vice versa.
    */
    template <class T>
    using vector = std::vector<T, allocator<T>>;
}


/**
 * @brief Copies the source vector to the destination vector.
*/
template <class T>
std::vector<T> operator<<=(std::vector<T>& dest, const ps::vector<T>& src) {
    dest.clear();

    for(auto it = src.begin(); it != src.end(); it++) {
        dest.push_back(*it);
    }

    return dest;
}

/**
 * @brief Copies the source vector to the destination vector.
*/
template <class T>
ps::vector<T> operator<<=(ps::vector<T>& dest, const std::vector<T>& src) {
    dest.clear();

    for(auto it = src.begin(); it != src.end(); it++) {
        dest.push_back(*it);
    }

    return dest;
}

/**
 * @brief Moves the source vector to the destination vector.
*/
template <class T>
std::vector<T> operator>>=(ps::vector<T>& dest, std::vector<T>& src) {
    dest.clear();

    dest <<= src;

    src.clear();
    return dest;
}

/**
 * @brief Moves the source vector to the destination vector.
*/
template <class T>
ps::vector<T> operator>>=(std::vector<T>& dest, ps::vector<T>& src) {
    dest.clear();

    dest <<= src;

    src.clear();
    return dest;
}

template <class T>
bool operator==(const std::vector<T>& lhs, const ps::vector<T>& rhs) {
    ps::vector<T> temp;
    temp <<= lhs;
    return (rhs == temp);
}

template <class T>
bool operator==(const ps::vector<T>& lhs, const std::vector<T>& rhs) {
    return rhs == lhs;
}

template <class T>
bool operator!=(const std::vector<T>& lhs, const ps::vector<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps::vector<T>& lhs, const std::vector<T>& rhs) {
    return !(lhs == rhs);
}

#endif