#pragma once

#ifndef MESSAGE_DESERIALIZER_H
#define MESSAGE_DESERIALIZER_H

#include <ArduinoJson.h>
#include "ps_string.h"
#include "json_allocator.h"

class MessageDeserializer {
    private:
        JsonDoc document;

        void deserialize_json(const ps_string& message);
        ps_string decompressString(const ps_string& message);

    public:
    MessageDeserializer(const ps_string& message);

    ps_string operator[] (const ps_string&);

};

#endif