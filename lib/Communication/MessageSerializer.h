#ifndef MESSAGE_SERIALIZER_H
#define MESSAGE_SERIALIZER_H

#include <stdlib.h>
#include <memory>

#include "../ps_stl/ps_stl.h"

#include <ArduinoJson.h>
#include "json_allocator.h"

#include "MQTTClient.h"

#include "MessageCompressor.h"

/**
 * @brief Creates a JSON document of requested size on construction. The JSON document can be modified during the lifetime of the class,
 * finally, when the class runs out of scope, any data in the JSON document is automatically serialized and sent using the MQTT Client to
 * the requested topic.
 */
class MessageSerializer : private MessageCompressor {
    private:
        std::shared_ptr<MQTTClient> mqtt_client;
        size_t topic;
          
    public:
        DynamicPSRAMJsonDocument document;
        MessageSerializer(std::shared_ptr<MQTTClient> client, size_t egress_topic, size_t json_document_size);
        ~MessageSerializer();
};

#endif