#include "MessageDeserializer.h"
#include "base64.h"

#include <../brotli/include/brotli/decode.h>

#include "esp_log.h"

#ifdef DEBUG_COMPRESSION
#define TAG_MESSAGE_DESERIALIZER "MESSAGE_DESERIALIZER"
#endif

MessageDeserializer::MessageDeserializer(const ps_string& message) : document(JsonDoc(JSON_DOCUMENT_SIZE)) {
    
#ifdef DEBUG_COMPRESSION
    uint64_t start_time = esp_timer_get_time();
#endif

    deserialize_json(message);

#ifdef DEBUG_COMPRESSION
    uint64_t tot_time = esp_timer_get_time() - start_time;

    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Deserialized JSON. [Took %uus]", tot_time);
#endif
}



void MessageDeserializer::deserialize_json(const ps_string& message) {
    JsonDoc temp_doc(JSON_DOCUMENT_SIZE);

    deserializeJson(temp_doc, message.c_str());

    temp_doc.shrinkToFit(); // Release unused memory back to allocator.

    ps_string decompress_str;
    if (temp_doc["enc"] == "br") {
        decompress_str <<= temp_doc["msg"];
        deserializeJson(document,  decompressString(decompress_str).c_str());
    } else {
        ESP_LOGI(TAG_MESSAGE_DESERIALIZER, "Deserialized Message: %s", temp_doc["msg"]);
        deserializeJson(document, temp_doc["msg"]);
    }
    
    document.shrinkToFit(); // Release unused memory back to allocator.
    document.garbageCollect();
}

/**
 * @brief Decompresses a string using the Brotli Algrotithm.
 * Dynamically allocates a buffer of BROTLI_DECODER_BUFFER_SIZE, uses the Google Brotli Decoder to decode the message
 * into the output buffer. Converts the output buffer to a ps_string and shrinks it to fit.
 * 
 * @note Returns an empty string on decompression failure
 * .
 * @param message Brotli Compressed string to decompress.
 * @return std::string - Decompressed Data
*/
ps_string MessageDeserializer::decompressString(const ps_string& message)
{
    size_t decoded_size = BROTLI_DECODER_BUFFER_SIZE;

    #ifdef DEBUG_COMPRESSION
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Decoding base64...");
    uint64_t start_time = esp_timer_get_time();
    uint64_t tot_time = 0;
    #endif

    ps_string inbuf = base64_decode(message);

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "base64 Decoded. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Allocating Memory...");
    start_time = esp_timer_get_time();
    #endif

    auto outbuf = (uint8_t*) heap_caps_calloc(1, sizeof(uint8_t) * decoded_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Memory Allocated. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Decompressing...");
    start_time = esp_timer_get_time();
    #endif

    BrotliDecoderResult status = BrotliDecoderDecompress(inbuf.size(), (const uint8_t *)inbuf.c_str(), &decoded_size, outbuf); // Decompress message with Brotli Decompression

    if (status != BROTLI_DECODER_RESULT_SUCCESS) // Check that decoder was successful.
    {
        ESP_LOGE(TAG_MESSAGE_DESERIALIZER, "Decompression Failed.");

        heap_caps_free(outbuf);

        return ps_string("");
    }

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Decompression Completed. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Converting to string and cleaning up...");
    start_time = esp_timer_get_time();
    #endif

    ps_string ret((char *)outbuf ); // Create a string containing the decompressed data.

    ret.shrink_to_fit(); // Ensure that the string is not oversized.

    heap_caps_free(outbuf);

    #ifdef DEBUG_COMPRESSION
    tot_time += esp_timer_get_time() - start_time;
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "Done. [Took %uus]", esp_timer_get_time() - start_time);
    ESP_LOGD(TAG_MESSAGE_DESERIALIZER, "\n==== Decompression Details ====\n- Decompression Time: %uus\n- Decompression Ratio: %f\n- Original Message: \'%s\'\n- Decompression Result: \'%s\'\n===============================", tot_time, (float)message.size() / (float)ret.size(), message.c_str(), ret.c_str());
    #endif

    return ret;
}

ps_string MessageDeserializer::operator[] (const ps_string& key) {
    ps_string ret;
    
    std::string value = document[key.c_str()];
    ret <<= value;

    return ret;
}