

#ifndef PS_STACK_H
#define PS_STACK_H

#include "ps_deque.h"
#include <stack>

template <class T>
using ps_stack = std::stack<T, ps_deque<T>>;

template <class T>
std::stack<T> operator<<=(std::stack<T>& dest, const ps_stack<T>& src) {
    ps_stack<T> temp = src;

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
ps_stack<T> operator<<=(ps_stack<T>& dest, const std::stack<T>& src) {
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
std::stack<T> operator>>=(std::stack<T>& dest, ps_stack<T>& src) {
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
ps_stack<T> operator>>=(ps_stack<T>& dest, std::stack<T>& src) {
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
bool operator==(const std::stack<T>& lhs, const ps_stack<T>& rhs) {
    ps_stack<T> temp;
    temp <<= lhs;
    return (rhs == temp);
}

template <class T>
bool operator==(const ps_stack<T>& lhs, const std::stack<T>& rhs) {
    return rhs == lhs;
}

template <class T>
bool operator!=(const std::stack<T>& lhs, const ps_stack<T>& rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator!=(const ps_stack<T>& lhs, const std::stack<T>& rhs) {
    return !(lhs == rhs);
}
#endif