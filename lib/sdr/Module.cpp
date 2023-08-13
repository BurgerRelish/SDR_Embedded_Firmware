#include "Module.h"


namespace sdr {
/**
 * @brief Saves the module parameters to the file opened by the Persistence class.
 * 
 * @param nvs 
 */
void Module::saveParameters(Persistence<fs::LittleFSFS>& nvs) {
    auto module_obj = nvs.document.createNestedObject();
    module_obj["mid"] = module_id.c_str();
    module_obj["pr"] = circuit_priority;
    auto tag_arr = module_obj["tags"].createNestedArray();

    auto rule_arr = module_obj["rules"].createNestedArray();

    save = false;
}

/**
 * @brief Serializes the readings into the provided array. Automatically appends a new object to the array.
 * 
 * @return bool true - If serialization was successful, or the class is empty, else false.
 */
bool Module::serializeReadings(JsonArray& reading_array) {
    if (!readings.size()) return true;

    auto document = reading_array.createNestedObject();

    document["moduleID"] = module_id.c_str();
    document["count"] = readings.size();

    /* Load calculated values into json arrays.*/
    {        
        auto voltage_array = document.createNestedArray("voltage");
        loadArray(getSummarization(&Reading::voltage), voltage_array);
    }

    {
        auto frequency_array = document.createNestedArray("frequency");
        loadArray(getSummarization(&Reading::frequency), frequency_array);
    }

    {
        auto apparent_power_array = document.createNestedArray("apparentPower");
        loadArray(getSummarization(&Reading::apparent_power), apparent_power_array);
    }

    {
        auto power_factor_array = document.createNestedArray("powerFactor");
        loadArray(getSummarization(&Reading::power_factor), power_factor_array);
    }


    {        
        double kwh_sum = 0;
        ps::vector<uint64_t> ts_vector;
        for (auto& reading : readings) {
            kwh_sum += reading.kwh_usage;
            ts_vector.push_back(reading.timestamp);
        }

        document["kwh"] = kwh_sum;

        std::sort(ts_vector.begin(), ts_vector.end());
        
        auto timestamp_arr = document.createNestedArray("timestamp");
        timestamp_arr.add(ts_vector.front());
        timestamp_arr.add(ts_vector.back());
    }

    readings.clear();
    return true;
}

/**
 * @brief Serializes all the status changes on the module. Clears the status change stack.
 * 
 * @return bool true - If serialization was successful, or the class is empty, else false.
 */
bool Module::serializeStatusChange(JsonArray& array) {
    if (!_status.size()) return true;

    auto document = array.createNestedObject();
    
    document["moduleID"] = module_id.c_str();
    auto state_arr = document.createNestedArray("state");
    auto ts_arr = document.createNestedArray("timestamp");

    while(!_status.empty()) {
        state_arr.add(_status.back().state);
        ts_arr.add(_status.back().timestamp);
        _status.pop_back();
    }

    return true;
}

/**
 * @brief Loads the provided update object into the class.
 * 
 * @note JSON Format:
 * {
 *  "action" : "replace" or "append",
 *  "priority" : int,
 *  "rules" : [{
 *      "priority" : int,
 *      "expression" : "string",
 *      "command" : "string"
 *      }, ...],
 *  "tags" : ["string", ...]
 * }
 * 
 * @param update_obj 
 */
void Module::loadUpdate(JsonObject& update_obj) {
    save = true;
    update = false;

    auto rule_arr = update_obj["rules"].as<JsonArray>();
    ps::vector<std::tuple<int, ps::string, ps::string>> rule_vect;
    for (auto rule : rule_arr) {
        rule_vect.push_back(
                std::make_tuple(
                rule["priority"].as<int>(),
                rule["expression"].as<ps::string>(),
                rule["command"].as<ps::string>()
                )
        );
    }

    auto tag_arr = update_obj["tags"].as<JsonArray>();
    ps::vector<ps::string> tag_vect;
    for (auto tag : tag_arr) {
        tag_vect.push_back(
            tag.as<ps::string>()
        );
    }

    if (update_obj["action"].as<ps::string>() == "replace") {
        re::RuleEngineBase::replace_rules(rule_vect);
        re::RuleEngineBase::replace_tag(tag_vect);
    } else {
        re::RuleEngineBase::add_rule(rule_vect);
        re::RuleEngineBase::add_tag(tag_vect);
    }

    circuit_priority = update_obj["priority"].as<int>();

    return;
}

/**
 * @brief Get the Statistical Summarization of the requested attribute.
 * 
 * @param attribute 
 * @return std::tuple<double, double, double, double> Mean, Maximum, IQR, Kurtosis
 */
std::tuple<double, double, double, double> Module::getSummarization(double Reading::* attribute) {
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

}