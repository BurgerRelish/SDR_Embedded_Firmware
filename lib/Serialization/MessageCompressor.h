#pragma once

#ifndef MESSAGE_COMPRESSOR_H
#define MESSAGE_COMPRESSOR_H

#include <ps_stl.h>


class MessageCompressor {
    public:
        ps::string decompressString(const ps::string& message);
        ps::string compressString(const ps::string& message);
};

#endif