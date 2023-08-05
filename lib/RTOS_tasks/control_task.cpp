#include "control_task.h"
#include "../SDRApp.h"

void interface_rx_callback(uint8_t interface, std::string message);

void controlTaskFunction(void* global_class) {
    try {
        auto app = (SDR::AppClass*) global_class;
        xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> control_task_semaphore);

        Serial1.begin(115200, SERIAL_8N1, U1_RXD, U1_TXD);
        Serial2.begin(115200, SERIAL_8N1, U2_RXD, U2_TXD);

        {   /* Global variables handled in own scope to ensure timeous semaphore return. */
            /* Create the hardware interface driver. */
            auto interface = app -> get_interface();
            if(interface.data() == nullptr) {
                interface.data() = new InterfaceMaster(&Serial1, U1_DIR, U1_SWI, &Serial2, U2_DIR, U2_SWI, interface_rx_callback);
            }

            if(!interface.data() -> assign_addresses()) {
                delete interface.data();
                interface.data() = nullptr;
                throw SDR::Exception("Address assignment failed.");
            }
        }
        

        while(1) {
            xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(app -> control_task_semaphore);

            {  /* Global variables handled in own scope to ensure timeous semaphore return. */
                auto interface = app -> get_interface();
                interface.data() -> loop();
            }

            
        }

    } catch (SDR::Exception &e) {
        ESP_LOGE("CTRL", "SDR Exception: %s", e.what());
    } catch (std::exception &e) {
        ESP_LOGE("CTRL", "Exception: %s", e.what());
    }

    vTaskDelete(NULL);
}