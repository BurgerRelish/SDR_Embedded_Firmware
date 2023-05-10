#include <ReadingStore.h>
#include <time.h>


ReadingStore::ReadingStore(std::string db_module_id)
{
    ReadingStore::db_module_id = db_module_id;
}

/**
 * @brief Destructor for the ReadingStore object.
 *
 * This destructor frees all of the readings stored in the ReadingStore object. It iterates over the linked list of
 * readings, starting with the most recent reading and working backwards, freeing each reading and updating the pointer
 * to the previous reading until there are no more readings left to free.
 *
 * @return None.
 */
ReadingStore::~ReadingStore()
{
    while(reading_store_pointer != NULL) // Free all stored readings
    {
        struct Reading * next = reading_store_pointer -> previous_reading;

        free(reading_store_pointer);

        reading_store_pointer = next;
    }

    return;
}


/**
 * @brief Get the ID of the DB metering module associated with this ReadingStore object.
 * @return const std::string The ID of the database module.
 */
const std::string ReadingStore::getModuleID()
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
    while(reading_store_pointer != NULL) // Free all stored readings
    {
        struct Reading * next = reading_store_pointer -> previous_reading;

        free(reading_store_pointer);

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
    struct Reading* new_reading = (Reading *) malloc(sizeof(Reading));

    if(!getLocalTime(&time_info))
    {
        ESP_LOGE("Failed to get time.");
        return false;
    }

    time(&now);

    new_reading -> timestamp = (uint32_t) now;
    new_reading -> voltage = voltage;
    new_reading -> frequency = frequency;
    new_reading -> apparent_power = apparent_power;
    new_reading -> phase_angle = phase_angle;
    new_reading -> energy_usage = energy_usage;

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
const std::string ReadingStore::getDataPacket()
{
    ESP_LOGI("Serializing Reading for Module %s", db_module_id.c_str());

    std::string ret = "";

    ret.append("{\"DID\":\"");
    ret.append(db_module_id);
    ret.append("\",\"R\":[");

    while(1) // Create JSON array of readings
    {
        char buffer[12] = {0};

        ret.append("{\"V\":"); // Append Voltage Measurement
        sprintf(buffer, "%3.3f", reading_store_pointer -> voltage);
        ret.append(buffer);

        ret.append(",\"F\":"); // Append Frequency Measurement
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%3.3f", reading_store_pointer -> frequency);
        ret.append(buffer);

        ret.append(",\"S\":"); // Append Apparent Power Measurement
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%3.3f", reading_store_pointer -> apparent_power);
        ret.append(buffer);

        ret.append(",\"A\":"); // Append Phase Angle Measurement
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "%3.3f", reading_store_pointer -> phase_angle);
        ret.append(buffer);

        ret.append(",\"E\":"); // Append Energy Usage Measurement
        memset(buffer, 0, sizeof(buffer)); 
        sprintf(buffer, "%3.3f", reading_store_pointer -> energy_usage);
        ret.append(buffer);

        ret.append(",\"T\":"); // Append Timestamp
        memset(buffer, 0, sizeof(buffer)); 
        sprintf(buffer, "%d", reading_store_pointer -> timestamp);
        ret.append(buffer);

        ret.append("}"); // Close Reading

        struct Reading * next_pointer = reading_store_pointer -> previous_reading; // Get location of previous reading and free the serialized reading.
        free(reading_store_pointer);

        if(next_pointer == NULL) // If there are no more readings to append, break;
        {
            reading_store_pointer = NULL;
            break;
        }
        else // Otherwise continue to serialize readings.
        {
            reading_store_pointer = next_pointer;
            ret.append(",");
        }
    }

    ret.append("]}");

    return ret;
}