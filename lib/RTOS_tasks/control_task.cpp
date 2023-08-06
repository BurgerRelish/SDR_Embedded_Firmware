#include "control_task.h"
#include "SDRApp.h"
#include <memory>
#include <unordered_map>
#include "../data_containers/ps_smart_ptr.h"
#include "../data_containers/ps_vector.h"
#include "../data_containers/ps_string.h"

struct InterfaceRxMessage {
    uint8_t interface;
    std::string message;
} incoming_message;

void interfaceRxCallback(uint8_t, std::string);
void handleControlMessage(std::shared_ptr<SDR::AppClass>, std::shared_ptr<InterfaceMaster>);

void controlTaskFunction(void* global_class) {
    std::shared_ptr<InterfaceMaster> interface(nullptr);

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

        { /* Create a module class for every discovered module. */
            auto filesys = app -> get_fs();
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/modules.txt", 4096, true); // Load existing Modules and their Tags, and allow for updates to be written to fs.
            auto known_modules = nvs.document.as<JsonArray>();

            std::unordered_map<std::string, std::pair<uint8_t, std::vector<std::string>>> module_map;

            /* Load all known modules into an unordered map for ease of identification. */
            for (size_t i = 0; i < known_modules.size(); i++) {
                std::vector<std::string> tags;
                auto tag_list = known_modules[i]["tags"].as<JsonArray>();

                for (auto tag : tag_list) {
                    tags.push_back(tag);
                }

                module_map.insert(
                    std::pair<std::string, std::pair<uint8_t, std::vector<std::string>>>(
                        known_modules[i]["mid"],
                        std::pair<uint8_t, std::vector<std::string>>(known_modules[i]["pr"].as<uint8_t>(),
                                                                    tags))
                );
            }

            incoming_message.interface = 3;
            for (uint8_t interface_no = 0; interface_no <= 1; interface_no++) {
                for (uint8_t address = 0; address <= interface -> get_end_device_address(interface_no); address++) {
                    interface -> send(interface_no, address, 1);
                    while (incoming_message.interface == 3) {vTaskDelay(5/portTICK_PERIOD_MS);} // Wait for module parameter announce.
                    auto modules = app -> get_modules(); // Fetch global module vector.
                    
                    auto found_module = module_map.find(incoming_message.message);

                    if (found_module == module_map.end()) { // Found unknown module
                        modules.data().push_back(
                                ps::make_shared<Module>(
                                    incoming_message.message, // ID
                                    std::vector<std::string>(), // Tag List
                                    0, // Priority
                                    address, // SWI Address
                                    interface_no, // RS485 Interface
                                    true // Requires an update.
                                )
                        );

                        auto new_module = nvs.document.createNestedObject();
                        new_module["mid"] = incoming_message.message.c_str();
                        new_module["pr"] = 0;
                        auto tag_arr = new_module["tags"].createNestedArray();

                    } else { // Found known module.
                        modules.data().push_back(
                            ps::make_shared<Module>(
                                found_module -> first, // ID
                                found_module -> second.second, // Tag List
                                found_module -> second.first, // Priority
                                address, // SWI Address
                                interface_no // RS485 Interface
                                ) // Does not require an update
                        );
                    }

                    incoming_message.interface = 3;
                }
            }
        }

        { /* Create the unit class */
            auto unit = app -> get_unit();
            auto filesys = app -> get_fs();
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/device.txt", 1024, true);

            std::vector<std::string> tag_list;
            auto tag_array = nvs.document["tags"].as<JsonArray>();

            for(auto v : tag_array) {
                tag_list.push_back(v.as<std::string>());
            }

            app -> set_unit(
                ps::make_shared<SDRUnit>(
                    nvs.document["uid"].as<std::string>(), // Unit ID
                    app -> get_modules().data().size(), // Module Count
                    tag_list // Unit Tag List
                )
            );
        }
        
        while(1) {
            /* Check that function has not been paused. */
            xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(app -> control_task_semaphore);
            
            /* Receive and Handle Queue messages. */
            handleControlMessage(app, interface);

            /* Service interface loop. */
            interface -> loop();
        }

    } catch (SDR::Exception &e) {
        ESP_LOGE("CTRL", "SDR Exception: %s", e.what());
    } catch (std::exception &e) {
        ESP_LOGE("CTRL", "Exception: %s", e.what());
    }

    vTaskDelete(NULL);
}

void interfaceRxCallback(uint8_t interface, std::string message) {
    incoming_message.interface = interface;
    incoming_message.message = message;
}


void handleControlMessage(std::shared_ptr<SDR::AppClass> app, std::shared_ptr<InterfaceMaster> interface) {

}

