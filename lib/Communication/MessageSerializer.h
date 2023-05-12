#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <Arduino.h>
#include <../brotli/decode.h>
#include <../brotli/encode.h>
#include <string>
#include <stdlib.h>

#include <../config.h>

class MessageSerializer
{
    public:
        MessageSerializer();
        ~MessageSerializer();

        std::string compressString(const std::string * message);
        std::string decompressString(const std::string * message);

};

#endif