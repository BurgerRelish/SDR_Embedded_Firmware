#include "ModuleInterface.h"

ModuleInterface::ModuleInterface(Stream* serial, uint8_t control_line, uint8_t dir_pin){
    current_address = 1; // Start addresses at 1.
    stream = serial;
    ctrl = control_line;
    dir = dir_pin;
}

ps::vector<std::pair<AnnouncePacket, uint8_t>> ModuleInterface::begin(){
    pinMode(dir, OUTPUT);
    pinMode(ctrl, OUTPUT);
    digitalWrite(dir, LOW); // RX dir
    digitalWrite(ctrl, HIGH); // Not addressing yet.

    ESP_LOGI("Addressing", "Creating Packets.");
    AddressPacket* address_packet = (AddressPacket*) calloc(1, sizeof(AddressPacket));
    transfer_out.begin((uint8_t*)address_packet, sizeof(AddressPacket), stream, 0, dir);
    
    AnnouncePacket* announce_packet = (AnnouncePacket*) calloc(1, sizeof(AnnouncePacket));
    transfer_in.begin((uint8_t*)announce_packet, sizeof(AnnouncePacket), stream, 0, dir);

    clearStreamBuffer();

    ESP_LOGI("Addressing", "Starting.");
    ps::vector<std::pair<AnnouncePacket, uint8_t>> ret;
    address_packet -> address = current_address;
    digitalWrite(ctrl, LOW); // Start Addressing.
    vTaskDelay(10 / portTICK_PERIOD_MS);
    while(receive()) {
        if (transmit(0)) { // Send address to module.
            ESP_LOGI("Addressing", "Found Module: %s", &(announce_packet -> id[0]));
            ret.push_back( 
                std::make_pair(
                    *announce_packet, address_packet->address
                )
            );
            address_packet->address ++;
        } else ESP_LOGE("TX", "Failed to send address.");
    }

    ESP_LOGI("Addressing", "Finished.");
    free(announce_packet);
    free(address_packet);
    digitalWrite(ctrl, HIGH); // End Addressing

    
    transfer_in = EasyTransfer();
    transfer_out = EasyTransfer();
    transfer_in.begin(details(reading_packet), stream, 0, dir); // Switch to receiving readings.
    transfer_out.begin(details(operation_packet), stream, 0, dir); // Switch to sending operations.
    
    clearStreamBuffer();

    return ret;
}


bool ModuleInterface::sendOperation(uint8_t address, uint16_t operation){
    operation_packet.operation = operation;

    return transmit(address);
}

/**
 * @brief Sends a read meter operation to the address provided, then returns the reading data.
 * @note voltage will be set to -1 of the reading fails.
 * @return The reading data packet.
*/
ReadingDataPacket ModuleInterface::getReading(uint8_t address){
    reading_packet = ReadingDataPacket();

    clearStreamBuffer();
    if (!sendOperation(address, OPERATION_READ_METER)) {
        reading_packet.voltage = -1;
        return reading_packet;
    }

    receive();

    return reading_packet;
}

void ModuleInterface::clearStreamBuffer() {
    while(stream -> available() ) {
        stream -> read();
    }
}

bool ModuleInterface::transmit(uint8_t address) {
    uint32_t start_tm = millis();
    while((millis() - start_tm) < 1000) {
        if (transfer_out.sendData(address)) return true;
        vTaskDelay(250 / portTICK_PERIOD_MS);
    };

    return false;
}

bool ModuleInterface::receive() {
    uint32_t start_tm = millis();
    while((millis() - start_tm) < 1000) {
        if (transfer_in.receiveData()) return true;
        vTaskDelay(25 / portTICK_PERIOD_MS);
    };

    return false;
}
