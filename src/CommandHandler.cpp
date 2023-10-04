#include "CommandHandler.h"
#include <ArduinoJson.h>
#include "Persistence.h"

CommandHandler::CommandHandler() {
}

void CommandHandler::begin(std::shared_ptr<Unit> unit, std::shared_ptr<Scheduler> scheduler) {
    this -> unit = unit;
    this -> scheduler = scheduler;
}

/**
 * @brief Handles the incoming command in the deserializer.
 * 
 * @param deserializer 
 */
void CommandHandler::handle(std::shared_ptr<MessageDeserializer> deserializer) {
    auto& message = deserializer -> document;

    auto command_type = message["type"].as<int32_t>();
    auto command = message["data"].as<JsonObject>();

    switch (command_type) {
        case 0: // RuleEngine Command
            handleRuleEngineCommand(command);
        break;

        case 1: // Scheduler Command
            handleSchedulerCommand(command);
        break;

        case 2: // Control Unit Parameters
            handleControlUnitParameters(command);
            break;
    }
}

/**
 * @brief Handles the incoming rule engine command.
 * 
 * @param object 
 */
void CommandHandler::handleRuleEngineCommand(JsonObject& object) {
    auto& module_map = unit -> module_map;

    auto unit_rules =  object["unit_rules"].as<JsonObject>();
    auto module_rules = object["module_rules"].as<JsonArray>();

    loadUnitRules(unit_rules);
    loadModuleRules(module_rules);
}

/**
 * @brief Load the unit rules from the JSON object.
 * 
 * @param unit_rules 
 */
void CommandHandler::loadUnitRules(JsonObject& unit_rules) {
    auto rule_action = unit_rules["action"].as<int32_t>();
    auto new_rules = unit_rules["rules"].as<JsonArray>();

    switch (rule_action)
    {
    case 0: // Append
        for (JsonObject new_rule : new_rules) {
            int32_t priority = new_rule["priority"].as<int32_t>();
            ps::string expression = new_rule["expression"].as<ps::string>();
            ps::string command = new_rule["command"].as<ps::string>();

            unit -> add_rule(priority, expression, command);
        }
        break;

    case 1: // Replace
        unit -> clear_rules();

        for (JsonObject new_rule : new_rules) {
            int32_t priority = new_rule["priority"].as<int32_t>();
            ps::string expression = new_rule["expression"].as<ps::string>();
            ps::string command = new_rule["command"].as<ps::string>();

            unit -> add_rule(priority, expression, command);
        }
        break;

    case 2: // Execute
        for (JsonObject new_rule : new_rules) {
            ps::string command = new_rule["command"].as<ps::string>();
            unit -> execute(command);
        }
        break;
    case 3: // Execute If
        for (JsonObject new_rule : new_rules) {
            int32_t priority = new_rule["priority"].as<int32_t>();
            ps::string expression = new_rule["expression"].as<ps::string>();
            ps::string command = new_rule["command"].as<ps::string>();

            unit -> execute_if(expression, command);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Load the module rules from the array.
 * 
 * @param module_rules 
 */
void CommandHandler::loadModuleRules(JsonArray& module_rules) {
    auto& module_map = unit -> module_map;

    for (JsonObject rule : module_rules) {
        auto module_id = rule["module_id"].as<ps::string>();
        auto module = module_map.find(module_id);

        if (module == module_map.end()) continue; // Skip Unknown modules.

        auto rule_action = rule["action"].as<int32_t>();
        auto new_rules = rule["rules"].as<JsonArray>();

        switch (rule_action)
        {
        case 0: // Append
            for (JsonObject new_rule : new_rules) {
                int32_t priority = new_rule["priority"].as<int32_t>();
                ps::string expression = new_rule["expression"].as<ps::string>();
                ps::string command = new_rule["command"].as<ps::string>();

                module -> second -> add_rule(priority, expression, command);
            }
            break;

        case 1: // Replace
            module -> second -> clear_rules();

            for (JsonObject new_rule : new_rules) {
                int32_t priority = new_rule["priority"].as<int32_t>();
                ps::string expression = new_rule["expression"].as<ps::string>();
                ps::string command = new_rule["command"].as<ps::string>();

                module -> second -> add_rule(priority, expression, command);
            }
            break;

        case 2: // Execute
            for (JsonObject new_rule : new_rules) {
                ps::string command = new_rule["command"].as<ps::string>();
                module -> second -> execute(command);
            }
            break;
        case 3: // Execute If
            for (JsonObject new_rule : new_rules) {
                int32_t priority = new_rule["priority"].as<int32_t>();
                ps::string expression = new_rule["expression"].as<ps::string>();
                ps::string command = new_rule["command"].as<ps::string>();

                module -> second -> execute_if(expression, command);
            }
            break;
        default:
            break;
        }
    }
}


/**
 * @brief Handles the incoming scheduler command. Skips unknown modules.
 * 
 * @param object 
 */
void CommandHandler::handleSchedulerCommand(JsonObject& object) {
    auto& module_map = unit -> module_map;
    auto action = object["action"].as<int32_t>();
    auto shedule_items = object["schedule"].as<JsonArray>();

    switch (action) {
        case 0: // Append
            break;

        case 1: // Replace
            scheduler -> clear();
            break;

        default:
            break;
    }

    for (JsonObject item : shedule_items) {
        ps::string module_id = item["module_id"].as<ps::string>();
        if (module_map.find(module_id) == module_map.end()) continue; // Skip unknown modules.

        scheduler -> add(SchedulerItem(item));
    }

}

/**
 * @brief Handles the incoming control unit parameters command.
 * 
 * @param object 
 */
void CommandHandler::handleControlUnitParameters(JsonObject& object) {
    unit -> sample_period = object["sample_period"].as<int32_t>();
    unit -> serialization_period = object["serialization_period"].as<int32_t>();
}