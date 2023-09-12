#include "ModuleInterface.h"

ModuleInterface::ModuleInterface(Stream* serial, uint8_t control_line, uint8_t dir_pin){

}

ps::vector<std::pair<AnnouncePacket, uint8_t>> ModuleInterface::begin(){
    pinMode(dir, OUTPUT);
    pinMode(ctrl, OUTPUT);
    digitalWrite(dir, LOW); // RX dir
    digitalWrite(ctrl, HIGH); // Not addressing yet.

    AddressPacket address_packet;
    transfer_out.begin(details(address_packet), stream, 0, dir);
    
    AnnouncePacket announce_packet;
    transfer_out.begin(details(announce_packet), stream, 0, dir);

    clearStreamBuffer();

    receive();

    



    digitalWrite(ctrl, LOW); // Start Addressing.
}


bool ModuleInterface::sendOperation(uint8_t address, uint16_t operation){

}

ReadingDataPacket ModuleInterface::getReading(uint8_t address){

}

void ModuleInterface::clearStreamBuffer() {
    while(stream -> available() ) {
        stream -> read();
    }
}

void ModuleInterface::transmit(uint8_t address) {
    while(!transfer_out.sendData(address)) {}
}

void ModuleInterface::receive() {
    while(!transfer_in.receiveData()) {}
}
