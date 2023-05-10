#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <Arduino.h>
#include <../brotli/decode.h>
#include <../brotli/encode.h>
#include <string>
#include <stdlib.h>

#define BROTLI_DECODER_BUFFER_SIZE 4096

class MessageSerializer
{
    public:
        MessageSerializer();
        ~MessageSerializer();

        const std::string compressString(std::string message);
        const std::string decompressString(std::string message);

    private:

    protected:

};

#endif