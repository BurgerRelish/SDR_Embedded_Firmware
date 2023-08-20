#pragma once

#ifndef STATUS_CHANGE_H
#define STATUS_CHANGE_H

#include <stdlib.h>

namespace sdr {

class StatusChange {
    public:
    uint64_t timestamp;
    bool status;
};

}

#endif