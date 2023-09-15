#pragma once

#ifndef PS_PRIORITY_QUEUE
#define PS_PRIORITY_QUEUE

#include <queue>
#include "ps_vector.h"

namespace ps {
    template<class T, typename Compare>
    using priority_queue = std::priority_queue<T, ps::vector<T>, Compare>;
}




#endif