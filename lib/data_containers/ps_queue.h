

#ifndef PS_QUEUE_H
#define PS_QUEUE_H

#include "ps_deque.h"
#include <queue>

template <class T>
using ps_queue = std::queue<T, ps_deque<T>>;

template <class T>
std::queue<T> operator<<=(std::queue<T>& dest, const ps_queue<T>& src) {
    ps_queue<T> temp = src;

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
ps_queue<T> operator<<=(ps_queue<T>& dest, const std::queue<T>& src) {
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
std::queue<T> operator>>=(std::queue<T>& dest, ps_queue<T>& src) {
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
ps_queue<T> operator>>=(ps_queue<T>& dest, std::queue<T>& src) {
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
bool operator==(const std::queue<T>& lhs, const ps_queue<T>& rhs) {
    ps_queue<T> temp;
    temp <<= lhs;
    return (rhs == temp);
}

template <class T>
bool operator==(const ps_queue<T>& lhs, const std::queue<T>& rhs) {
    return rhs == lhs;
}

template <class T>
bool operator!=(const std::queue<T>& lhs, const ps_queue<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps_queue<T>& lhs, const std::queue<T>& rhs) {
    return !(lhs == rhs);
}

#endif