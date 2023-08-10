#include "control_task.h"
#include "SDRApp.h"
#include <memory>
#include <unordered_map>
#include <tuple>

#include "../ps_stl/ps_stl.h"

struct InterfaceRxMessage {
    uint16_t address;
    std::string message;
};

std::shared_ptr<ps::queue<InterfaceRxMessage>> incoming_message;


using ModuleParameterMap = std::unordered_map<std::string, std::tuple<int, std::vector<std::string>, std::vector<Rule>>>;
using ModuleAddressMap = std::unordered_map<uint16_t, std::shared_ptr<Module>>;

void interfaceRxCallback(uint16_t, std::string);
void handleControlMessage(std::shared_ptr<SDR::AppClass>&, std::shared_ptr<InterfaceMaster>&);
void handleInterfaceMessage(std::shared_ptr<SDR::AppClass>&, InterfaceRxMessage&, ModuleAddressMap&);
void sendStateChanges(std::shared_ptr<SDR::AppClass>&, std::shared_ptr<InterfaceMaster>&);

ModuleParameterMap loadModuleParameters(Persistence<fs::LittleFSFS>&);

uint16_t countChar(std::string, char);

void controlTaskFunction(void* global_class) {
    std::shared_ptr<InterfaceMaster> interface(nullptr);
    incoming_message = std::make_shared<ps::queue<InterfaceRxMessage>>();
    
    std::shared_ptr<SDR::AppClass> app(nullptr);
    { /* Convert nullptr into AppClass pointer, then get a shared pointer of the class, releasing the appClass pointer once it is no longer required. */
        auto appClass = static_cast<SDR::AppClass*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    /* Wait until task is ready to run. */
    xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> control_task_semaphore);
    app -> setStatusLEDState(STATUS_LED_DISCOVERING_MODULES);
    
    try {
        /* Init RS485 Interfaces. */
        Serial1.begin(115200, SERIAL_8N1, U1_RXD, U1_TXD);
        Serial2.begin(115200, SERIAL_8N1, U2_RXD, U2_TXD);

        {   /* Create the hardware interface driver. */
            interface = ps::make_shared<InterfaceMaster>(&Serial1, U1_DIR, U1_SWI, &Serial2, U2_DIR, U2_SWI, interfaceRxCallback);

            if(!interface -> assign_addresses()) {
                throw SDR::Exception("Address assignment failed.");
            }
        }


        ModuleAddressMap module_address_map;

        { /* Create a module class for every discovered module. */
            auto filesys = app -> get_fs(); // Get lock on global variables.
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/modules.txt", 4096, true); // Load existing Modules and their Tags, and allow for updates to be written to fs.
            auto module_map = loadModuleParameters(nvs);

            for (uint8_t interface_no = 0; interface_no <= 1; interface_no++) {
                for (uint8_t address = 0; address <= interface -> get_end_device_address(interface_no); address++) {
                    interface -> send(interface_no, address, 1);
                    while (incoming_message->empty()) {vTaskDelay(5/portTICK_PERIOD_MS);} // Wait for module parameter announce.
                    auto modules = app -> get_modules(); // Fetch global module vector.
                    
                    auto found_module = module_map.find(incoming_message->front().message);

                    if (found_module == module_map.end()) { // Found unknown module
                        modules.data().push_back(
                                ps::make_shared<Module>(                // Create a new Module class on PSRAM.
                                    incoming_message->front().message,   // ID
                                    -1,                                 // Priority
                                    std::vector<std::string>(),         // Tag List
                                    std::vector<Rule>(),                // Rule List
                                    address,                            // SWI Address
                                    interface_no,                       // RS485 Interface
                                    true                                // Requires an update.
                                )
                        );
                    } else { // Found known module.
                        modules.data().push_back(
                            ps::make_shared<Module>(                 // Create a new Module class on PSRAM.
                                found_module -> first,               // ID
                                std::get<0>(found_module -> second), // Priority
                                std::get<1>(found_module -> second), // Tag List
                                std::get<2>(found_module -> second), // Rule List
                                address,                             // SWI Address
                                interface_no                         // RS485 Interface
                                )                                    // Does not require an update
                        );
                    }

                    /* Store module address in */
                    module_address_map.insert(
                        std::make_pair(
                            address + ((interface) ? 256 : 0),      // Offset interface_1 by 256.
                            modules.data().back()                   // Get shared_ptr of module.
                        )
                    );

                    incoming_message->pop();
                }
            }
        } // Return SDR::VarGuard semaphores.

        /* Generate the module map. */
        app -> generate_module_map();

        { /* Create the unit class */
            auto unit = app -> get_unit(); // Get lock on global variables.
            auto filesys = app -> get_fs();
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/device.txt", 1024, true); // Open unit document.

            /* Load tag list */
            std::vector<std::string> tag_list;
            {            
                auto tag_array = nvs.document["tags"].as<JsonArray>();
                for(auto v : tag_array) {
                    tag_list.push_back(v.as<std::string>());
                }
            }

            /* Load rule list */
            std::vector<Rule> rule_list;
            {
                auto rule_array = nvs.document["rules"].as<JsonArray>();
                for (auto rule : rule_array) {
                    Rule new_rule {
                        rule["pr"].as<int>(),
                        rule["exp"].as<ps::string>(),
                        rule["cmd"].as<ps::string>()
                    };
                    rule_list.push_back(new_rule);
                }
            }


            /* Create a new unit class and update the global variable of it. */
            app -> set_unit(
                ps::make_shared<SDRUnit>(
                    nvs.document["uid"].as<std::string>(), // Unit ID
                    app -> get_modules().data().size(), // Module Count
                    tag_list, // Unit Tag List
                    rule_list
                )
            );
        } // Return SDR::VarGuard semaphores.
        
        /* Notify Sentry Task that setup has completed. */
        {
            SentryQueueMessage msg;
            msg.new_state = CTRL_SETUP_COMPLETE;
            msg.data = nullptr;
            if(xQueueSend(app -> sentry_task_queue, (void*) &msg, portMAX_DELAY) != pdTRUE) throw SDR::Exception("Failed to send message.");
            vTaskSuspend(NULL);
        }
        
        while(1) {
            /* Check that function has not been paused. */
            xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(app -> control_task_semaphore);
            
            /* Receive and Handle Queue messages. */
            handleControlMessage(app, interface);

            /* Transmit any state changes. */
            sendStateChanges(app, interface);

            /* Service interface loop to receive messages. */
            interface -> loop();

            /* Check for any messages received. */
            if (!incoming_message->empty()) {
                handleInterfaceMessage(app, incoming_message->front(), module_address_map);
                incoming_message->pop();
            }
        }

    } catch (SDR::Exception &e) {
        ESP_LOGE("CTRL", "SDR Exception: %s", e.what());
    } catch (std::exception &e) {
        ESP_LOGE("CTRL", "Exception: %s", e.what());
    }

    vTaskDelete(NULL);
}

/**
 * @brief Callback for the InterfaceMaster class. Adds the received message and its interface onto the incoming message queue.
 * 
 * @param address 
 * @param message 
 */
void interfaceRxCallback(uint16_t address, std::string message) {
    incoming_message->push(
        InterfaceRxMessage {
            address, 
            message
        }
    );

    return;
}

void handleInterfaceMessage(std::shared_ptr<SDR::AppClass>& app, InterfaceRxMessage& message, ModuleAddressMap& module_map) {
    auto modules_guard = app -> get_modules(); // Take control of modules semaphore.

    auto module_pair = module_map.find(message.address);
    
    if (module_pair == module_map.end()) throw SDR::Exception("Unknown address received.");
    auto module = module_pair -> second;

    switch (countChar(message.message, '|')) {
        case 6: // Reading: "V|PF|AP|RP|SP|kWh|State"
        {        
            size_t pos = 0;
            ps::queue<double> variables;
            while(pos < message.message.size()) {
                size_t new_pos = message.message.find_first_of('|', pos);
                if (new_pos == std::string::npos) new_pos = message.message.length();

                variables.push(std::atof(message.message.substr(pos, new_pos - pos).c_str()));
                pos = ++new_pos; // Skip "|"
            }

            module -> addReading(
                Reading(variables, app -> get_epoch_time())
            );
        }
        break;
        case 0: // Command Result: "Result"
            module -> newStatusChange(
                StatusChange{
                    (std::atof(message.message.c_str()) == 1) ? true : false,
                    app -> get_epoch_time()
                }
            );

        break;
    }
}

void handleControlMessage(std::shared_ptr<SDR::AppClass>& app, std::shared_ptr<InterfaceMaster>& interface) {
    
    ControlQueueMessage msg;

}

/**
 * @brief Transmits any relay state changes made to any Module to the requested module.
 * 
 * @param app 
 * @param interface 
 */
void sendStateChanges(std::shared_ptr<SDR::AppClass>& app, std::shared_ptr<InterfaceMaster>& interface) {
    auto modules_var = app -> get_modules();

    for (auto module : modules_var.data()) {
        if (!module -> statusChanged()) continue;
        interface -> send(
            module -> offset(),
            module -> address(),
            (module -> status()) ? 31 : 30 // ON / OFF
        );
    }
}

/**
 * @brief Loads the module parameters from flash into an unordered map for module discovery.
 * 
 * @param nvs 
 * @return ModuleParameterMap 
 */
ModuleParameterMap loadModuleParameters(Persistence<fs::LittleFSFS>& nvs) {
    auto known_modules = nvs.document.as<JsonArray>();

    ModuleParameterMap module_map;

    for (auto module : known_modules) {
        std::vector<std::string> tags;
        {     /* Load tags into vector */
            auto tag_list = module["tags"].as<JsonArray>();
            for (auto tag : tag_list) {
                tags.push_back(tag);
            }
        }

        /* Load rules into vector */
        std::vector<Rule> rules;
        {
            auto rule_list = module["rules"].as<JsonArray>();
            for (auto rule : rule_list) {
                Rule new_rule {
                    rule["pr"].as<int>(),
                    rule["exp"].as<ps::string>(),
                    rule["cmd"].as<ps::string>()
                };

                rules.push_back(new_rule);
            }
        }

        /* Insert new module into map */
        module_map.insert(
            std::make_pair (
                module["mid"].as<std::string>(), // Module ID
                std::make_tuple(
                    module["pr"].as<int>(), // Module Priority
                    tags, // Module Tag List
                    rules // Module Rule List
                )
            )
        );

    }

    return module_map;
}

/**
 * @brief Counts the number of occurences of a char in a string.
 * 
 * @param message string to count chars in.
 * @param ch char to count.
 * @return uint16_t number of matching chars.
 */
uint16_t countChar(std::string message, char ch) {
    uint16_t count = 0;
    for (auto str_ch : message) {
        if(str_ch == ch) count++;
    }

    return count;
}



