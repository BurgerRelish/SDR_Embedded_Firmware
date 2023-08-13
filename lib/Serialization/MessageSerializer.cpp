#include "MessageSerializer.h"

/**
 * @brief Construct a new Message Serializer:: Message Serializer object. The JSON Document is automatically serialized, compressed, and sent using the provided MQTT Client to the
 * server on the provided topic number when this class is destructed/runs out of scope.
 * 
 * @param client 
 * @param egress_topic 
 * @param json_document_size 
 */
MessageSerializer::MessageSerializer(std::shared_ptr<MQTTClient> client, size_t egress_topic, size_t json_document_size) :
    mqtt_client(client),
    topic(egress_topic),
    document(DynamicPSRAMJsonDocument(json_document_size))
{
}

MessageSerializer::~MessageSerializer() {
    ps::string compressed_str;
    {
        ps::string uncompressed_str;
        if(!serializeJson(document, uncompressed_str)) {
            ESP_LOGV("SERIALIZER", "Document empty.");
            return;
        }
        document.clear();
        compressed_str = brotli::compress(uncompressed_str);
    }

    document["enc"] = "br";
    document["msg"] = compressed_str.c_str();

    compressed_str.clear();

    if(!serializeJson(document, compressed_str)) {
        ESP_LOGE("SERIALIZER", "Serialization failed.");
    }

    if(!mqtt_client -> send(compressed_str, topic)) {
        ESP_LOGE("SERIALIZER", "MQTT Publish failed.");
    }
}
