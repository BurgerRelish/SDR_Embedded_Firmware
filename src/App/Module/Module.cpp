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

    if (readings.size() > 300) readings.pop_back();

    return true;
}

/**
 * @brief Serializes the readings into the provided array. Automatically appends a new object to the array.
 * 
 * @return bool true - If serialization was successful, or the class is empty, else false.
 */
bool Module::serialize(JsonObject& obj) {
    // Fetch all the new readings.
    ps::deque<Reading> new_reading_deque;
    auto it = readings.cbegin();
    for (uint16_t i = 0; i < new_readings; i++, it++) {
        new_reading_deque.push_back(*it);
    }

    obj[JSON_MODULE_UID].set(module_id.c_str());
    obj[JSON_READING_COUNT].set(new_reading_deque.size());

    { // Sum the kWh Usage readings.
        double kwh_usage = 0;
        for (auto reading : new_reading_deque) {
            kwh_usage += reading.kwh_usage;
        }
        obj[JSON_KWH_USAGE].set(kwh_usage);
    }


    // Load the mean voltage
    obj[JSON_VOLTAGE].set(calc_mean<double>(&Reading::voltage, new_reading_deque));
    
    // Load the mean frequency
    obj[JSON_FREQUENCY].set(calc_mean<double>(&Reading::frequency, new_reading_deque));
    
    { // Load the apparent power features
        auto apparent_power = get_summary(&Reading::apparent_power, new_reading_deque);
        auto apparent_arr = obj.createNestedArray(JSON_APPARENT_POWER);
        apparent_arr.add(std::get<0>(apparent_power)); // Mean
        apparent_arr.add(std::get<1>(apparent_power)); // Maximum
        apparent_arr.add(std::get<2>(apparent_power)); // IQR

        auto kurt = std::get<3>(apparent_power);
        apparent_arr.add((!isnan(kurt)) ? kurt : 0); // Kurtosis 
    }

    { // Load the power factor features
        auto power_factor = get_summary(&Reading::power_factor, new_reading_deque);
        auto pf_arr = obj.createNestedArray(JSON_POWER_FACTOR);
        pf_arr.add(std::get<0>(power_factor)); // Mean
        pf_arr.add(std::get<1>(power_factor)); // Maximum
        pf_arr.add(std::get<2>(power_factor)); // IQR

        auto kurt = std::get<3>(power_factor);
        pf_arr.add((!isnan(kurt)) ? kurt : 0); // Kurtosis 
    }
   
    { // Serialize state changes.
        JsonArray state_changes_array = obj.createNestedArray("state_changes");
        auto it = status_updates.cbegin();
        for (uint16_t i = 0; i < new_status_changes; i++, it++) {
            JsonObject change_obj = state_changes_array.createNestedObject();
            change_obj["state"] = it -> status;
            change_obj["timestamp"] = it -> timestamp;
        }

        new_status_changes = 0;
    }

    new_readings = 0;
    return true;
}

/**
 * @brief Saves the class into the provided array.
 * 
 * @param update_obj 
 */
bool Module::save(JsonObject& object) {
    if (object.containsKey(JSON_MODULE_UID)) 
        if (object[JSON_MODULE_UID].as<ps::string>() != module_id) 
            return false;
    
    auto rule_obj = object["rule_engine"].as<JsonObject>();
    save_rule_engine(rule_obj);

    return true;
}

/**
 * @brief Loads the provided update object into the class.
 * 
 * @param update_obj 
 */
bool Module::load(JsonObject& object) {
    if (object[JSON_MODULE_UID].as<ps::string>() != module_id) return false;
    
    auto rule_obj = object["rule_engine"].as<JsonObject>();
    load_rule_engine(rule_obj);

    return true;
}

/**
 * @brief Get the Statistical Summarization of the requested attribute from the new readings.
 * 
 * @param attribute 
 * @return std::tuple<double, double, double, double> Mean, Maximum, IQR, Kurtosis
 */
std::tuple<double, double, double, double> Module::get_summary(double Reading::* attribute,  ps::deque<Reading>& readings) {
    auto mean = calc_mean<double>(attribute, readings);
    return std::make_tuple(
        mean,
        calc_max<double>(attribute, readings),
        calc_iqr<double>(attribute, readings),
        calc_kurt<double>(mean, attribute, readings)
    );
}

void Module::load_re_vars() {
    re::RuleEngineBase::set_var(re::VAR_CLASS, MODULE_CLASS, (void*) this);

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
    re::RuleEngineBase::set_var(re::VAR_INT, READING_COUNT, std::function<int()>([this](){ return this -> getReadings().size(); }));
    re::RuleEngineBase::set_var(re::VAR_INT, NEW_READING_COUNT, std::function<int()>([this](){ return this -> new_readings; }));
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
    if (!status_updates.empty()) {
        if (state == status_updates.front().status) return true;
    }

    StatusChange new_change;
    new_change.status = state;
    

    if (!interface -> sendOperation(slave_address, (state) ? OPERATION_RELAY_SET : OPERATION_RELAY_RESET)) return false;
    new_status_changes++;
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
