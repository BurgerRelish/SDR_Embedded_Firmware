#pragma once

#ifndef MESSAGE_DESERIALIZER_H
#define MESSAGE_DESERIALIZER_H

#include <ArduinoJson.h>
#include "../ps_stl/ps_stl.h"

#include "MessageCompressor.h"
#include "json_allocator.h"

/**
 * @brief Automatically decompresses and deserializes the JSON formatted string. The JSON document can be used to get any data contained in the message. 
 * 
 */
class MessageDeserializer: private MessageCompressor {
    private:
        void deserialize_json(const ps::string& message);
    public:
        DynamicPSRAMJsonDocument document;
        MessageDeserializer(const ps::string& message);

};

#endif