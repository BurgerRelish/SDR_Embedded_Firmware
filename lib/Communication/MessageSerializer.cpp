#include "MessageSerializer.h"

#include <Arduino.h>
#include "ps_base64.h"
#include <cctype>
#include <algorithm>
#include "esp_heap_caps.h"

#include <../brotli/include/brotli/encode.h>


#ifdef DEBUG_COMPRESSION
#define TAG_MESSAGE_SERIALIZER "MESSAGE_SERIALIZER"
#endif

MessageSerializer::MessageSerializer(const std::shared_ptr<SDRUnit> unit, const ps_vector<std::shared_ptr<Module>> modules) : _unit(unit), _modules(modules), document(DynamicJsonDocument(JSON_DOCUMENT_SIZE)) {}

/**
 * @brief Serializes all readings and status updates for all modules into a JSON packet, compresses it using brotli, and places it into
 * a transmission packet.
 * @return ps_string JSON formatted string ready for transmission.
*/
ps_string MessageSerializer::serializeReadings() {
#ifdef DEBUG_COMPRESSION
    uint64_t start_time = esp_timer_get_time();
#endif

    document.clear();
    document["type"].set("reading");

    auto reading_list = document.createNestedArray("reading");
    auto status_updates = document.createNestedArray("statusUpdates");

    for (size_t i = 0; i < _modules.size(); i++) {
        auto readings = _modules.at(i) -> getReadings();

        while (!readings.empty()) {
            JsonObject reading_data = reading_list.createNestedObject();

            reading_data["moduleID"].set(_modules.at(i) -> id().c_str());
            reading_data["voltage"].set(readings.top().voltage);
            reading_data["frequency"].set(readings.top().frequency);
            reading_data["apparentPower"].set(readings.top().apparent_power);
            reading_data["powerFactor"].set(readings.top().power_factor);
            reading_data["kwh"].set(readings.top().kwh_usage);
            reading_data["timestamp"].set(readings.top().timestamp);

            readings.pop();
        }

        if (_modules.at(i) -> statusChanged()) {
            ps_stack<StatusChange> changes = _modules.at(i) -> getStatusChanges();

            while (!changes.empty()) {
                JsonObject status_change = status_updates.createNestedObject();
                status_change["moduleID"].set(_modules.at(i) -> id().c_str());
                status_change["state"].set(changes.top().state);
                status_change["timestamp"].set(changes.top().timestamp);

                changes.pop();
            }
        }
    }

    document.shrinkToFit();
    ps_string to_compression;
    serializeJson(document, to_compression);

    ps_string compressed_str = compressString(to_compression);

    /* Pack the compressed message into a new message ready for transmission. */
    document.clear();
    document["enc"].set("br"),
    document["msg"].set(compressed_str.c_str());

    ps_string retval;

    serializeJson(document, retval);
    document.clear();
    document.shrinkToFit();

#ifdef DEBUG_COMPRESSION
    uint64_t tot_time = esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Serialized Message. [Took %uus]", tot_time);
#endif

    return retval;
}

/**
 * @brief Serializes an update request packet. The serialized message is compressed, before being placed in a transmission packet.
 * @param ps_vector<Module*> Vector containing a pointer to the module classes for which an update is required.
 * @return ps_string JSON formatted string ready for transmission.
*/
ps_string MessageSerializer::serializeUpdateRequest(const ps_vector<Module*> modules) {
#ifdef DEBUG_COMPRESSION
    uint64_t start_time = esp_timer_get_time();
#endif

    document.clear();
    document["type"].set("update");
    auto id_array = document.createNestedArray("moduleID");

    for (size_t i = 0; i < modules.size(); i++) {
        id_array.add(modules.at(i) -> id().c_str());
    }

    document.shrinkToFit();

    ps_string to_compression;
    serializeJson(document, to_compression);

    ps_string compressed_str = compressString(to_compression);

    /* Pack the compressed message into a new message ready for transmission. */
    document.clear();
    document["enc"].set("br"),
    document["msg"].set(compressed_str.c_str());

    ps_string retval;

    serializeJson(document, retval);
    document.clear();
    document.shrinkToFit();

#ifdef DEBUG_COMPRESSION
    uint64_t tot_time = esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Serialized Message. [Took %uus]", tot_time);
#endif

    return retval;
}

ps_string MessageSerializer::serializeNotification(ps_string notification) {
#ifdef DEBUG_COMPRESSION
    uint64_t start_time = esp_timer_get_time();
#endif

    document.clear();
    document["type"].set("notification");
    document["msg"].set(notification.c_str());

    ps_string to_compression;

    serializeJson(document, to_compression);

    document.clear();
    document["enc"].set("br");
    document["msg"].set(compressString(to_compression).c_str());

    ps_string retval;
    serializeJson(document, retval);

    document.clear();
    document.shrinkToFit();

#ifdef DEBUG_COMPRESSION
    uint64_t tot_time = esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Serialized Message. [Took %uus]", tot_time);
#endif

    return retval;
}

/**
 * @brief Compresses a string using the Brotli Algorithm.
 * Dynamically allocates a temporary buffer to store the compressed string. Uses the 
 * Google Brotli Encoder to compress the inout string. Copys the output of the encoder to a 
 * new string and shrinks it to fit.
 * 
 * @note Returns an empty string on compression failure.
 * 
 * @param message std::string to be compressed.
 * @return std::string - Compressed string data.
*/
ps_string MessageSerializer::compressString(const ps_string& message)
{
    #ifdef DEBUG_COMPRESSION
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Allocating Memory...");
    uint64_t start_time = esp_timer_get_time();
    uint64_t tot_time = 0;
    #endif

    size_t data_size = message.size();
    size_t outbytes = data_size + 150;
    auto outbuf = (uint8_t*) heap_caps_calloc(1, sizeof(uint8_t) * outbytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Memory Allocated. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Starting Compression...");
    start_time = esp_timer_get_time();
    #endif

    int status = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_MODE_TEXT, data_size, (const uint8_t *)message.c_str(), &outbytes, outbuf); // Compress message with Brotli Compression

    if (status != BROTLI_TRUE) // Check that encoder was successful.
    {
        ESP_LOGE(TAG_MESSAGE_SERIALIZER, "Compression Failed.");
        heap_caps_free(outbuf);
        return ps_string("");
    }

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Compression completed. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Encoding in base64...");
    start_time = esp_timer_get_time();
    #endif

    ps_string ret = base64_encode((unsigned char *)outbuf, outbytes, false); // Create a string containing the compressed data.
    heap_caps_free(outbuf);

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "Encoding completed. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_SERIALIZER, "\n==== Compression Details ====\n- Compression Time: %uus\n- Compression Ratio: %f\n- Original Message: \'%s\'\n- Compression Result: \'%s\'\n=============================", tot_time, (float)message.size() / (float)ret.size(), message.c_str(), ret.c_str());
    #endif

    return ret;
}
