#include "EasyTransfer.h"


//Captures address and size of struct
void EasyTransfer::begin(uint8_t * ptr, uint8_t length, Stream *theStream, uint8_t id, uint8_t dir_pin){
	address = ptr;
	size = length;
	_stream = theStream;

  device_id = id;
	_pin = dir_pin;
	//dynamic creation of rx parsing buffer in RAM
	rx_buffer = (uint8_t*) malloc(size+1);
}

//Sends out struct in binary, with header, length info and checksum
bool EasyTransfer::sendData(uint8_t target_id){
  uint8_t CS = size;
  digitalWrite(_pin, HIGH);
  delayMicroseconds(10);
  _stream->write(0x06);
  _stream->write(target_id);
  _stream->write(size);
  _stream->write(0x85);

  for(int i = 0; i<size; i++){
    CS^=*(address+i);
    _stream->write(*(address+i));
  }
  _stream->write(CS);

  _stream->flush();
  digitalWrite(_pin, LOW);

  uint32_t start_tm = micros();
  while(_stream -> read() != 0xFE && micros() - start_tm < EASYTRANSFER_TIMEOUT_US) {}

  if (micros() - start_tm < EASYTRANSFER_TIMEOUT_US) return true;
  return false;
}

bool EasyTransfer::receiveData() {
  uint8_t buffer[128] = {0};

  if (_stream->available() < 3) {
    return false;
  }
  delay(5);
  //Serial.write(_stream->available());
  uint8_t len = 0;
  while(Serial.available() > 0 && len < 127) {
    buffer[len] = Serial.read();
    len++;
  }

  uint8_t start_index = 0;
  for (uint8_t i = 0; i < len; i++) {
    if (buffer[i] == 0x06) {
      start_index = i;
      break;
    }

    if (i == len) return false;
  }

  delay(5);


  if (buffer[start_index + 1] != device_id) return false;

  if (buffer[start_index + 3] != 0x85) {
    return false;
  }

  rx_len = buffer[start_index + 2];

// l  1  2  3  4  5  6  7  8
  // 06 04 85 01 00 01 00 04

  //make sure the binary structs on both Arduinos are the same size.
  if(rx_len != size){
    rx_len = 0;
    return false;
  }

  calc_CS = rx_len;
  for (int i = start_index; i < rx_len; i++){
    calc_CS^=buffer[i + start_index + 4];
  } 
  
  if(calc_CS == buffer[start_index + rx_len + 4]){//CS good
    memcpy(address,&buffer[start_index + 4], size);
    rx_len = 0;
    rx_array_inx = 0;
    digitalWrite(_pin, HIGH);
    delayMicroseconds(10);
    Serial.write(0xFE); // Write ACK.
    Serial.flush();
    digitalWrite(_pin, LOW);
    return true;
  }

  //failed checksum, need to clear this out anyway
  rx_len = 0;
  rx_array_inx = 0;
  return false;
}

