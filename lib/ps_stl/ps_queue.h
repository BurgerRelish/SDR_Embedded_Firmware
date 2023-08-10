

#ifndef PS_QUEUE_H
#define PS_QUEUE_H

#include "ps_deque.h"
#include <queue>

namespace ps {
template <class T>
using queue = std::queue<T, ps::deque<T>>;
}

template <class T>
std::queue<T> operator<<=(std::queue<T>& dest, const ps::queue<T>& src) {
    ps::queue<T> temp = src;

    while(!dest.empty()) {
        dest.pop();
    }

    while (!temp.empty()) {
        dest.push(temp.front());
        temp.pop();
    }

    return dest;
}

template <class T>
ps::queue<T> operator<<=(ps::queue<T>& dest, const std::queue<T>& src) {
    std::queue<T> temp = src;

    while(!dest.empty()) {
        dest.pop();
    }

    while (!temp.empty()) {
        dest.push(temp.front());
        temp.pop();
    }

    return dest;
}

template <class T>
std::queue<T> operator>>=(std::queue<T>& dest, ps::queue<T>& src) {
    while(!dest.empty()) {
        dest.pop();
    }

    while (!src.empty()) {
        dest.push(src.front());
        src.pop();
    }

    return dest;
}

template <class T>
ps::queue<T> operator>>=(ps::queue<T>& dest, std::queue<T>& src) {
    while(!dest.empty()) {
        dest.pop();
    }

    while (!src.empty()) {
        dest.push(src.front());
        src.pop();
    }

    return dest;
}

template <class T>
bool operator==(const std::queue<T>& lhs, const ps::queue<T>& rhs) {
    ps::queue<T> temp;
    temp <<= lhs;
    return (rhs == temp);
}

template <class T>
bool operator==(const ps::queue<T>& lhs, const std::queue<T>& rhs) {
    return rhs == lhs;
}

template <class T>
bool operator!=(const std::queue<T>& lhs, const ps::queue<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps::queue<T>& lhs, const std::queue<T>& rhs) {
    return !(lhs == rhs);
}

#endif