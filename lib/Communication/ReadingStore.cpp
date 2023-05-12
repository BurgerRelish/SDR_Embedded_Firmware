#include <ReadingStore.h>

#include <string>
#include <iostream>
#include <sstream> // for ostringstream
#include <time.h>

ReadingStore::ReadingStore(const std::string db_module_id) : db_module_id(db_module_id) {}

/**
 * @brief Destructor for the ReadingStore object.= 
 *
 * This destructor frees all of the readings stored in the ReadingStore object. It iterates over the linked list of
 * readings, starting with the most recent reading and working backwards, freeing each reading and updating the pointer
 * to the previous reading until there are no more readings left to free.
 *
 * @return None.
 */
ReadingStore::~ReadingStore()
{
    while(reading_store_pointer != nullptr) // Free all stored readings
    {
        struct Reading * next = reading_store_pointer -> previous_reading;

        delete reading_store_pointer;

        reading_store_pointer = next;
    }

    return;
}


/**
 * @brief Get the ID of the DB metering module associated with this ReadingStore object.
 * @return const std::string The ID of the database module.
 */
std::string ReadingStore::getModuleID() const
{
    return db_module_id;
}


/**
 * @brief Deletes all readings contained within the ReadingStore object.
 *
 * This function frees all of the readings stored in the ReadingStore object. It iterates over the linked list of
 * readings, starting with the most recent reading and working backwards, freeing each reading and updating the pointer
 * to the previous reading until there are no more readings left to free.
 *
 * @return None.
 */
bool ReadingStore::clean()
{
    while(reading_store_pointer != nullptr) // Free all stored readings
    {
        struct Reading * next = reading_store_pointer -> previous_reading;

        delete reading_store_pointer;

        reading_store_pointer = next;
    }

    return true;
}

/**
 * @brief Add a new reading to the ReadingStore object.
 *
 * This function adds a new reading to the ReadingStore object with the specified voltage, frequency, apparent power,
 * phase angle, energy usage, and timestamp values. The new reading is added to the beginning of the linked list of
 * readings stored in the ReadingStore object.
 *
 * @param voltage The voltage measurement value for the new reading.
 * @param frequency The frequency measurement value for the new reading.
 * @param apparent_power The apparent power measurement value for the new reading.
 * @param phase_angle The phase angle measurement value for the new reading.
 * @param energy_usage The energy usage measurement value for the new reading.
 *
 * @return True if the reading was successfully added, false otherwise.
 */
bool ReadingStore::addReading(double voltage, double frequency, double apparent_power, double phase_angle, double energy_usage)
{
    time_t now;
    struct tm time_info;

    auto new_reading = new Reading;
    std::ostringstream retval;

    if(!getLocalTime(&time_info))
    {
        ESP_LOGE("Failed to get time.");
        return false;
    }

    time(&now);

    retval 
    << "{\"V\":" << voltage << ","
    << "\"F\":" << frequency << ","
    << "\"S\":" << apparent_power << ","
    << "\"PA\":" << phase_angle << ","
    << "\"E\":" << energy_usage << ","
    << "\"TS\":" << now
    << "}";

    new_reading -> message = retval.str();

    new_reading -> previous_reading = reading_store_pointer;

    reading_store_pointer = new_reading;

    return true;
}


/**
 * @brief Serialize the readings stored in the ReadingStore object and return them as a JSON string.
 *
 * This function serializes the readings stored in the ReadingStore object and returns them as a JSON string. The
 * readings are serialized in the following format:
 *   {
 *     "DID": "<module_id>",
 *     "R": [
 *       {
 *         "V": <voltage_measurement>,
 *         "F": <frequency_measurement>,
 *         "S": <apparent_power_measurement>,
 *         "A": <phase_angle_measurement>,
 *         "E": <energy_usage_measurement>,
 *         "T": <timestamp>
 *       },
 *       ...
 *     ]
 *   }
 * where <module_id> is the ID of the module associated with the ReadingStore object, and <voltage_measurement>,
 * <frequency_measurement>, <apparent_power_measurement>, <phase_angle_measurement>, <energy_usage_measurement>,
 * and <timestamp> are the corresponding measurement values for each reading stored in the ReadingStore object.
 * 
 * @return The serialized readings as a JSON string.
 */
std::string ReadingStore::getDataPacket()
{
    ESP_LOGI("Serializing Reading for Module %s", db_module_id.c_str());

    std::ostringstream retval;

    retval
    << R"({"DID":")" << db_module_id << "\","
    << R"("R":[)";

    while(true) // Create JSON array of readings
    {
        retval << reading_store_pointer -> message;

        auto next_pointer = reading_store_pointer -> previous_reading; // Get location of previous reading and free the serialized reading.
        
        delete reading_store_pointer;

        if(next_pointer == nullptr) // If there are no more readings to append, break;
        {
            reading_store_pointer = nullptr;
            break;
        }
        else // Otherwise continue to serialize readings.
        {
            reading_store_pointer = next_pointer;
            retval << ",";
        }
    }

    retval << "]}";

    return retval.str();
}