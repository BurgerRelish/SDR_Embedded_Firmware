#include "Module.h"


namespace sdr {

/**
 * @brief Serializes the readings into the provided array. Automatically appends a new object to the array.
 * 
 * @return bool true - If serialization was successful, or the class is empty, else false.
 */
bool Module::serialize(JsonArray& reading_array) {
    if (new_readings.size() == 0) return true;
    
    auto obj = reading_array.createNestedObject();

    obj[JSON_MODULE_ID] = module_id.c_str();
    obj[JSON_READING_COUNT] = readings.size();

    double kwh_usage = 0;
    for (auto reading : new_readings) {
        kwh_usage += reading.kwh_usage;
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
        tm_arr.add(new_readings.begin()->timestamp);
        tm_arr.add(new_readings.back().timestamp);
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
    load_rule_engine(update_obj);
}

/**
 * @brief Get the Statistical Summarization of the requested attribute.
 * 
 * @param attribute 
 * @return std::tuple<double, double, double, double> Mean, Maximum, IQR, Kurtosis
 */
std::tuple<double, double, double, double> Module::get_summary(double Reading::* attribute) {
    auto mean = calc_mean<double>(attribute, new_readings);
    return std::make_tuple(
        mean,
        calc_max<double>(attribute, new_readings),
        calc_iqr<double>(attribute, new_readings),
        calc_kurt<double>(mean, attribute, new_readings)
    );
}

void Module::load_re_vars() {
    std::shared_ptr<Module> this_ptr = std::enable_shared_from_this<Module>::shared_from_this();
    re::RuleEngineBase::set_var(re::VAR_CLASS, MODULE_CLASS, this_ptr);

    re::RuleEngineBase::set_var(re::VAR_DOUBLE, ACTIVE_POWER, std::function<double()>([this]() { return this->latestReading().active_power; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, REACTIVE_POWER, std::function<double()>([this]() { return this->latestReading().reactive_power; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, APPARENT_POWER, std::function<double()>([this]() { return this->latestReading().apparent_power; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, VOLTAGE, std::function<double()>([this]() { return this->latestReading().voltage; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, FREQUENCY, std::function<double()>([this]() { return this->latestReading().frequency; }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, POWER_FACTOR, std::function<double()>([this]() { return this->latestReading().power_factor; }));
    re::RuleEngineBase::set_var(re::VAR_UINT64_T, SWITCH_TIME, std::function<uint64_t()>([this]() { return this->switchTime(); }));
    re::RuleEngineBase::set_var(re::VAR_INT, CIRCUIT_PRIORITY, std::function<int()>([this]() { return this->priority(); }));
    re::RuleEngineBase::set_var(re::VAR_STRING, MODULE_ID, std::function<ps::string()>([this]() { return this->id(); }));
    re::RuleEngineBase::set_var(re::VAR_ARRAY, MODULE_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
    re::RuleEngineBase::set_var(re::VAR_BOOL, SWITCH_STATUS, std::function<bool()>([this]() { return this->status(); }));
}

    const ps::string& Module::id() {
        return module_id;
    }

    const int& Module::priority() {
        return circuit_priority;
    }

    const uint8_t& Module::address() {
        return swi_address;
    }

    const uint8_t& Module::offset() {
        return io_offset;
    }

    const bool& Module::status() {
        return status_updates.front().status;
    }

    const uint64_t& Module::switch_time() {
        return status_updates.front().timestamp;
    }

    /**
     * @brief Get the Status Changes object. Will be empty after serializeStatusChanges() has been called,
     * 
     * @return ps::vector<StatusChange>& 
     */
    ps::vector<StatusChange>& Module::get_status_changes() {
        return status_updates;
    }

    void Module::state_changed(bool new_state) {

    }

    void Module::new_status_change(StatusChange new_status) {
        status_updates.push_back(new_status);
    }

    void Module::set_relay_state(bool state) {

    }

    /**
     * @brief Adds a new reading to the module vector. Pops a reading off the back if the deque is too big.
    */
    void Module::add_reading(const Reading& reading) {
        readings.emplace_front(reading);
        new_readings.emplace_front(reading);

        if (readings.size() > READING_DEQUE_SIZE) {
            readings.pop_back();
        }
    }

    /**
     * @brief Get the latest reading object.
     * 
     * @return const Reading& 
     */
    const Reading& Module::latestReading() {
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
        return update;
    }
    
    /**
     * @brief Check whether the module parameters require saving.
     * 
     * @return true 
     * @return false 
     */
    bool& Module::saveRequired() {

    }

}