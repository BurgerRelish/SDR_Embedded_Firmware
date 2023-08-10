#pragma once

#ifndef PS_PRIORITY_QUEUE
#define PS_PRIORITY_QUEUE

#include <queue>
#include "ps_vector.h"

namespace ps {
    template<class T>
    class Compare {
        public:
            /* Descending Order Comparison Operator */
            bool operator()(const T& a, const T& b) {
                return (a.priority < b.priority);
            }
    };

    template<class T>
    using priority_queue = std::priority_queue<T, ps::vector<T>, Compare<T>>;
}




#endif