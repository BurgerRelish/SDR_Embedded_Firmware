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

    AddressPacket address_packet;
    transfer_out.begin(details(address_packet), stream, 0, dir);
    
    AnnouncePacket announce_packet;
    transfer_in.begin(details(announce_packet), stream, 0, dir);

    clearStreamBuffer();

    digitalWrite(ctrl, LOW); // Start Addressing.
    ps::vector<std::pair<AnnouncePacket, uint8_t>> ret;

    address_packet.address = current_address;

    while(receive()) {
        if (transmit(0)) { // Send address to module.
            address_packet.address = current_address++;
            ret.push_back( 
                std::make_pair(
                    announce_packet, current_address
                )
            );
        } else ESP_LOGE("TX", "Failed to send address.");
    }

    digitalWrite(ctrl, HIGH); // End Addressing

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
    uint16_t attempts = 0;
    while(!transfer_out.sendData(address) && attempts < 100) attempts++;

    if (attempts >= 100 ) return false;
    return true;
}

bool ModuleInterface::receive() {
    uint16_t attempts = 0;
    while(!transfer_in.receiveData() && attempts < 100) attempts++;

    if (attempts >= 100 ) return false;
    return true;
}
