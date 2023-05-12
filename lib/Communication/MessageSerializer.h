#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <Arduino.h>
#include <../config.h>

#include <../brotli/decode.h>
#include <../brotli/encode.h>
#include <string>
#include <stdlib.h>



class MessageSerializer
{
    public:
        std::string compressString(const std::string * message);
        std::string decompressString(const std::string * message);

};

#endif