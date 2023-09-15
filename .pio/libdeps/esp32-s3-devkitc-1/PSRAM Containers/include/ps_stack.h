

#ifndef PS_STACK_H
#define PS_STACK_H

#include "ps_deque.h"
#include <stack>
namespace ps {
template <class T>
using stack = std::stack<T, deque<T>>;
}


template <class T>
std::stack<T> operator<<=(std::stack<T>& dest, const ps::stack<T>& src) {
    ps::stack<T> temp = src;

    while(!dest.empty()) {
        dest.pop();
    }

    while (!temp.empty()) {
        dest.push(temp.top());
        temp.pop();
    }

    return dest;
}

template <class T>
ps::stack<T> operator<<=(ps::stack<T>& dest, const std::stack<T>& src) {
    std::stack<T> temp = src;

    while(!dest.empty()) {
        dest.pop();
    }

    while (!temp.empty()) {
        dest.push(temp.top());
        temp.pop();
    }

    return dest;
}

template <class T>
std::stack<T> operator>>=(std::stack<T>& dest, ps::stack<T>& src) {
    while(!dest.empty()) {
        dest.pop();
    }

    while (!src.empty()) {
        dest.push(src.top());
        src.pop();
    }

    return dest;
}

template <class T>
ps::stack<T> operator>>=(ps::stack<T>& dest, std::stack<T>& src) {
    while(!dest.empty()) {
        dest.pop();
    }

    while (!src.empty()) {
        dest.push(src.top());
        src.pop();
    }

    return dest;
}

template <class T>
bool operator==(const std::stack<T>& lhs, const ps::stack<T>& rhs) {
    ps::stack<T> temp;
    temp <<= lhs;
    return (rhs == temp);
}

template <class T>
bool operator==(const ps::stack<T>& lhs, const std::stack<T>& rhs) {
    return rhs == lhs;
}

template <class T>
bool operator!=(const std::stack<T>& lhs, const ps::stack<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps::stack<T>& lhs, const std::stack<T>& rhs) {
    return !(lhs == rhs);
}
#endif