#ifndef READINGSTORE_H
#define READINGSTORE_H

#include <Arduino.h>
#include <../config.h>

struct Reading
{
    std::string message;
    Reading * previous_reading = nullptr;
};

/**
 * @brief A class for storing and managing readings from a power monitoring module.
 *
 * The ReadingStore class provides a mechanism for storing readings from a power monitoring module. Readings are stored
 * in a linked list, with each reading pointing to the previous reading in the list. The most recent reading is stored
 * at the head of the list. The ReadingStore class also provides methods for adding new readings to the list, cleaning
 * up the list, and serializing the readings into a data packet of JSON format.
 * 
 * Readings are cleared once serialized.
 */
class ReadingStore
{
    public:
        explicit ReadingStore(std::string db_module_id);
        ~ReadingStore();
        
        bool addReading(double voltage, double frequency, double apparent_power, double phase_angle, double energy_usage);
        std::string getModuleID() const;
        std::string getDataPacket();
        bool clean();

    private:
        struct Reading * reading_store_pointer = nullptr;
        std::string db_module_id;
};

#endif