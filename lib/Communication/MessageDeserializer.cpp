#include "MessageDeserializer.h"

#include "esp_log.h"

#ifdef DEBUG_COMPRESSION
#define TAG_MESSAGE_DESERIALIZER "MESSAGE_DESERIALIZER"
#endif

MessageDeserializer::MessageDeserializer(const ps::string& message) : document(DynamicPSRAMJsonDocument(JSON_DOCUMENT_SIZE)) {
    
#ifdef DEBUG_COMPRESSION
    uint64_t start_time = esp_timer_get_time();
#endif

    deserialize_json(message);

#ifdef DEBUG_COMPRESSION
    uint64_t tot_time = esp_timer_get_time() - start_time;

    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Deserialized JSON. [Took %uus]", tot_time);
#endif
}



void MessageDeserializer::deserialize_json(const ps::string& message) {
    try {
        DynamicPSRAMJsonDocument temp_doc(JSON_DOCUMENT_SIZE);

        deserializeJson(temp_doc, message.c_str());

        ps::string decompress_str;
        if (!temp_doc.containsKey("enc")) throw;

        if (temp_doc["enc"] == "br") {
            decompress_str <<= temp_doc["msg"];
            deserializeJson(document,  decompressString(decompress_str).c_str());
        } else {
            ESP_LOGE(TAG_MESSAGE_DESERIALIZER, "Unknown message encoding. Ignoring...");
        }
    } catch (std::exception &e) {
        ESP_LOGI(TAG_MESSAGE_DESERIALIZER, "Direct message.");
        deserializeJson(document, message);
    }
}

