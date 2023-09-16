#include "Module.h"
#include <time.h>

uint64_t Module::getTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        //Serial.println("Failed to obtain time");
        return 0;
    }

    return (uint64_t) mktime(&timeinfo);
}

/**
 * @brief Gets a new reading from the assosciated meter module. Automatically adds the timestamp.
 * @return false if failed to get time or reading, true if successful.
*/
bool Module::refresh() {
    auto now = getTime();
    if (now == 0) return false;

    auto data = interface -> getReading(slave_address);
    if (data.voltage == -1) return false;

    Reading new_reading(data, (uint64_t) now);

    readings.push_front(new_reading);
    new_readings++;

    return true;
}

/**
 * @brief Serializes the readings into the provided array. Automatically appends a new object to the array.
 * 
 * @return bool true - If serialization was successful, or the class is empty, else false.
 */
bool Module::serialize(JsonArray& reading_array) {
    if (new_readings == 0) return true;
    
    auto obj = reading_array.createNestedObject();

    obj[JSON_MODULE_ID] = module_id.c_str();
    obj[JSON_READING_COUNT] = readings.size();

    double kwh_usage = 0;
    auto it = readings.cbegin();
    for (uint16_t i = 0; i < new_readings && it != readings.cend(); i++, it++) {
        kwh_usage += (*it).kwh_usage;
    }

    obj[JSON_KWH_USAGE] = kwh_usage;

    {
        auto voltage = get_summary(&Reading::voltage);
        auto volt_arr = obj.createNestedArray(JSON_VOLTAGE);
        volt_arr.add(std::get<0>(voltage)); // Mean
        volt_arr.add(std::get<1>(voltage)); // Maximum
        volt_arr.add(std::get<2>(voltage)); // IQR
        volt_arr.add(std::get<3>(voltage)); // Kurtosis
    }

    {
        auto frequency = get_summary(&Reading::frequency);
        auto freq_arr = obj.createNestedArray(JSON_VOLTAGE);
        freq_arr.add(std::get<0>(frequency)); // Mean
        freq_arr.add(std::get<1>(frequency)); // Maximum
        freq_arr.add(std::get<2>(frequency)); // IQR
        freq_arr.add(std::get<3>(frequency)); // Kurtosis
    }
    
    {
        auto apparent_power = get_summary(&Reading::apparent_power);
        auto apparent_arr = obj.createNestedArray(JSON_VOLTAGE);
        apparent_arr.add(std::get<0>(apparent_power)); // Mean
        apparent_arr.add(std::get<1>(apparent_power)); // Maximum
        apparent_arr.add(std::get<2>(apparent_power)); // IQR
        apparent_arr.add(std::get<3>(apparent_power)); // Kurtosis
    }

    {
        auto power_factor = get_summary(&Reading::power_factor);
        auto pf_arr = obj.createNestedArray(JSON_VOLTAGE);
        pf_arr.add(std::get<0>(power_factor)); // Mean
        pf_arr.add(std::get<1>(power_factor)); // Maximum
        pf_arr.add(std::get<2>(power_factor)); // IQR
        pf_arr.add(std::get<3>(power_factor)); // Kurtosis
    }
   
    {
        auto tm_arr = obj.createNestedArray(JSON_TIMESTAMP);
        tm_arr.add(readings.begin()->timestamp);
        tm_arr.add(readings.at(new_readings - 1).timestamp);
    }

    {    
        auto status_obj = obj.createNestedObject(JSON_STATUS_OBJ);
        auto status_arr = status_obj.createNestedArray(JSON_STATUS);
        auto status_ts = status_obj.createNestedArray(JSON_TIMESTAMP);
        for (auto status : status_updates) {
            status_arr.add(status.status);
            status_ts.add(status.timestamp);
        }
    }
    
    return true;
}

/**
 * @brief Saves the class into the provided object.
 * 
 * @param update_obj 
 */
void Module::save(JsonObject& update_obj) {
    update_obj[JSON_MODULE_ID] = module_id;
    update_obj[JSON_PRIORITY] = circuit_priority;

    auto reading_obj = update_obj[JSON_READING_OBJECT].as<JsonObject>();
    reading_obj[JSON_NEW_READINGS] = new_readings;

    auto voltage_arr = reading_obj[JSON_VOLTAGE].as<JsonArray>();
    auto frequency_arr = reading_obj[JSON_FREQUENCY].as<JsonArray>();
    auto apparent_power_arr = reading_obj[JSON_APPARENT_POWER].as<JsonArray>();
    auto power_factor_arr = reading_obj[JSON_POWER_FACTOR].as<JsonArray>();
    auto kwh_usage_arr = reading_obj[JSON_KWH_USAGE].as<JsonArray>();
    auto timestamp_arr = reading_obj[JSON_TIMESTAMP].as<JsonArray>();

    for (auto reading : readings) {
        voltage_arr.add(reading.voltage);
        frequency_arr.add(reading.frequency);
        apparent_power_arr.add(reading.apparent_power);
        kwh_usage_arr.add(reading.kwh_usage);
        timestamp_arr.add(reading.timestamp);
    }

    save_rule_engine(update_obj);
}

/**
 * @brief Loads the provided update object into the class.
 * 
 * @param update_obj 
 */
void Module::load(JsonObject& update_obj) {
    module_id = update_obj[JSON_MODULE_ID].as<ps::string>();
    circuit_priority = update_obj[JSON_PRIORITY];

    auto reading_obj = update_obj[JSON_READING_OBJECT].as<JsonObject>();
    new_readings = reading_obj[JSON_NEW_READINGS].as<uint16_t>();

    auto voltage_arr = reading_obj[JSON_VOLTAGE].as<JsonArray>();
    auto frequency_arr = reading_obj[JSON_FREQUENCY].as<JsonArray>();
    auto apparent_power_arr = reading_obj[JSON_APPARENT_POWER].as<JsonArray>();
    auto power_factor_arr = reading_obj[JSON_POWER_FACTOR].as<JsonArray>();
    auto kwh_usage_arr = reading_obj[JSON_KWH_USAGE].as<JsonArray>();
    auto timestamp_arr = reading_obj[JSON_TIMESTAMP].as<JsonArray>();

    auto voltage_it = voltage_arr.begin();
    auto freq_it = frequency_arr.begin();
    auto sp_it = apparent_power_arr.begin();
    auto pf_it = power_factor_arr.begin();
    auto kwh_it = kwh_usage_arr.begin();
    auto ts_it = timestamp_arr.begin();

    while(voltage_it != voltage_arr.end()) {
        readings.push_back(
            Reading (
                voltage_it -> as<double>(),
                freq_it -> as<double>(),
                sp_it -> as<double>(),
                pf_it -> as<double>(),
                kwh_it -> as<double>(),
                ts_it -> as<uint64_t>()
            )
        );

        voltage_it += 1;
        freq_it += 1;
        sp_it += 1;
        pf_it += 1;
        kwh_it += 1;
        ts_it += 1;
    }

    voltage_arr.clear();
    frequency_arr.clear();
    apparent_power_arr.clear();
    power_factor_arr.clear();
    kwh_usage_arr.clear();
    timestamp_arr.clear();

    load_rule_engine(update_obj);
}

void Module::loadFromArray(JsonArray& array) {
    for (auto item : array) {
        if (item.containsKey(JSON_MODULE_ID)) {
            if (item[JSON_MODULE_ID].as<ps::string>() == module_id) {
                auto item_obj = item.as<JsonObject>();
                load(item_obj);
                break;
            }
        }
    }
}

void Module::saveToArray(JsonArray& array) {
    auto save_obj = array.createNestedObject();
    save(save_obj);
}


/**
 * @brief Get the Statistical Summarization of the requested attribute from the new readings.
 * 
 * @param attribute 
 * @return std::tuple<double, double, double, double> Mean, Maximum, IQR, Kurtosis
 */
std::tuple<double, double, double, double> Module::get_summary(double Reading::* attribute) {
    ps::deque<Reading> to_serialize;
    auto it = readings.cbegin();
    for (uint16_t i = 0; i < new_readings && it != readings.cend(); i++, it++) {
        to_serialize.push_back(*it);
    }

    auto mean = calc_mean<double>(attribute, to_serialize);
    return std::make_tuple(
        mean,
        calc_max<double>(attribute, to_serialize),
        calc_iqr<double>(attribute, to_serialize),
        calc_kurt<double>(mean, attribute, to_serialize)
    );
}

void Module::load_re_vars() {
    //std::shared_ptr<Module> this_ptr = std::enable_shared_from_this<Module>::shared_from_this();
    //re::RuleEngineBase::set_var(re::VAR_CLASS, MODULE_CLASS, this);

    re::RuleEngineBase::set_var(re::VAR_DOUBLE, ACTIVE_POWER, std::function<double()>([this]() { return this->getLatestReading().active_power(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, REACTIVE_POWER, std::function<double()>([this]() { return this->getLatestReading().reactive_power(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, APPARENT_POWER, std::function<double()>([this]() { return this->getLatestReading().apparent_power; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, VOLTAGE, std::function<double()>([this]() { return this->getLatestReading().voltage; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, FREQUENCY, std::function<double()>([this]() { return this->getLatestReading().frequency; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, POWER_FACTOR, std::function<double()>([this]() { return this->getLatestReading().power_factor; }));
    re::RuleEngineBase::set_var(re::VAR_UINT64_T, SWITCH_TIME, std::function<uint64_t()>([this]() { return this->getRelayStateChangeTime(); }));
    re::RuleEngineBase::set_var(re::VAR_INT, CIRCUIT_PRIORITY, std::function<int()>([this]() { return this->getModulePriority(); }));
    re::RuleEngineBase::set_var(re::VAR_STRING, MODULE_ID, std::function<ps::string()>([this]() { return this->getModuleID(); }));
    re::RuleEngineBase::set_var(re::VAR_ARRAY, MODULE_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
    re::RuleEngineBase::set_var(re::VAR_BOOL, SWITCH_STATUS, std::function<bool()>([this]() { return this->getRelayState(); }));
}

const ps::string& Module::getModuleID() {
    return module_id;
}

const int& Module::getModulePriority() {
    return circuit_priority;
}

const bool Module::getRelayState() {
    if (status_updates.size() > 0)
        return status_updates.front().status;
    return false;
}

const uint64_t Module::getRelayStateChangeTime() {
    if (status_updates.size() > 0)
        return status_updates.front().timestamp;
    return 0;
}

/**
 * @brief Get the Status Changes object. Will be empty after serializeStatusChanges() has been called,
 * 
 * @return ps::vector<StatusChange>& 
 */
const ps::deque<StatusChange>& Module::getRelayStateChanges() {
    return status_updates;
}

/**
 * @brief Set the state of the relay on the attached slave device.
*/
bool Module::setRelayState(bool state) {
    StatusChange new_change;
    new_change.status = state;

    if (!interface -> sendOperation(slave_address, (state) ? OPERATION_RELAY_SET : OPERATION_RELAY_RESET)) return false;
    
    new_change.timestamp = getTime();
    status_updates.emplace_front(new_change);

    return true;
}

/**
 * @brief Get the latest reading object.
 * 
 * @return const Reading& 
 */
const Reading& Module::getLatestReading() {
    return readings.front();
}

/**
 * @brief Get the Readings object deque. May be empty after serialization, rather use `latestReading()` to get the latest reading.
 * 
 * @return ps::deque<Reading>& Reference to reading deque.
 */
const ps::deque<Reading>& Module::getReadings() {
    return readings;
}

/**
 * @brief Check whether the module requires an update.
 * 
 * @return true - Update required.
 * @return false - Update not required.
 */
bool& Module::updateRequired() {
    return update_required;
}

/**
 * @brief Check whether the module parameters require saving.
 * 
 * @return true 
 * @return false 
 */
bool& Module::saveRequired() {
    return save_required;
}
