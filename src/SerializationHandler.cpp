#include "SerializationHander.h"
#include "JSONFields.h"

SerializationHandler::SerializationHandler() {
    unit = nullptr;
}

void SerializationHandler::begin(std::shared_ptr<Unit> unit, std::shared_ptr<MQTTClient> mqtt_client) {
    this -> unit = unit;
    this -> mqtt_client = mqtt_client;
}

/**
 * @brief If it is time to serialize readings, serialize them and send them to the MQTT client.
 * 
 * @note This function will block if the MQTT client is not connected and the outgoing message queue is full.
 * 
 */
void SerializationHandler::serializeReadings() { 
    if (unit -> getTimeSinceLastSerialization() < unit -> serialization_period) return; // Not time to serialize readings yet.

    try {
        auto new_message = mqtt_client -> createMessage(0, 16 * 1024);

        new_message -> document[JSON_TYPE].set(0); // Set message type to reading.
        JsonObject data_obj = new_message -> document.createNestedObject(JSON_DATA);

        // Save period time data.
        std::pair<uint64_t, uint64_t> period = unit -> getSerializationPeriod();
        data_obj[JSON_PERIOD_START].set(period.first);
        data_obj[JSON_PERIOD_END].set(period.second);

        // Serialize modules into array.
        JsonArray data_array = data_obj.createNestedArray(JSON_READING_OBJECT);

        auto& modules = unit -> getModules();
        for (auto module : modules) {
        ESP_LOGD("Serialize", "Module: %s", module -> getModuleID().c_str());
        JsonObject obj = data_array.createNestedObject();
        module -> serialize(obj); // Get each module to add its reading data.
        };

        // Message will be sent as new_message class goes out of scope now. Note: Queue insertion with portMAX_DELAY.
    } catch (...) {
        ESP_LOGE("Unit", "Failed to serialize readings.");
    }
}
