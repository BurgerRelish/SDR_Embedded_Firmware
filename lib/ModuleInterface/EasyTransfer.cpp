#include "EasyTransfer.h"


//Captures address and size of struct
void EasyTransfer::begin(uint8_t * ptr, uint8_t length, Stream *theStream, uint8_t id, uint8_t dir_pin){
	address = ptr;
	size = length;
	_stream = theStream;

  device_id = id;
  pinMode(dir_pin, OUTPUT);
	_pin = dir_pin;
	//dynamic creation of rx parsing buffer in RAM
  heap_caps_free(rx_buffer);
	rx_buffer = (uint8_t*) heap_caps_malloc(size+1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

//Sends out struct in binary, with header, length info and checksum
bool EasyTransfer::sendData(uint8_t target_id){
  uint8_t CS = size;
  ESP_LOGI("EasyTransfer", "Sending Packet to address %d", target_id);
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
  ESP_LOGE("EasyTransfer", "ACK Timeout.");
  return false;
}

bool EasyTransfer::receiveData() {
  //start off by looking for the header bytes. If they were already found in a previous call, skip it.
  if(rx_len == 0){
  //this size check may be redundant due to the size check below, but for now I'll leave it the way it is.
    if(_stream->available() >= 3){
      //this will block until a 0x06 is found or buffer size becomes less then 3.
      while(_stream->read() != 0x06) {
      //This will trash any preamble junk in the serial buffer
      //but we need to make sure there is enough in the buffer to process while we trash the rest
      //if the buffer becomes too empty, we will escape and try again on the next call
        if(_stream->available() < 3) {
          ESP_LOGI("EasyTransfer", "Buffer Empty.");
          return false;
        } 
		  }
      if (_stream -> read() == device_id) {
        ESP_LOGI("EasyTransfer", "ID matched.");
        rx_len = _stream->read();
        if (_stream->read() == 0x85){
        //make sure the binary structs on both Arduinos are the same size.
          if(rx_len != size){
            ESP_LOGE("EasyTransfer", "Size Mismatch %x:%x.", rx_len, size);
            rx_len = 0;
            return false;
          }
        } else return false;   
      } else return false;    
    } else return false;
  }
  
  //we get here if we already found the header bytes, the struct size matched what we know, and now we are byte aligned.
  if(rx_len != 0){
    while(_stream->available() && rx_array_inx <= rx_len){
      rx_buffer[rx_array_inx++] = _stream->read();
    }
    
    if(rx_len == (rx_array_inx-1)){
      //seem to have got whole message
      //last uint8_t is CS
      calc_CS = rx_len;
      for (int i = 0; i<rx_len; i++){
        calc_CS^=rx_buffer[i];
      } 
      
      if(calc_CS == rx_buffer[rx_array_inx-1]){//CS good
        memcpy(address,rx_buffer,size);
        digitalWrite(_pin, HIGH);
        delayMicroseconds(10);
        _stream->write(0xFE); // Write ACK.
        _stream->flush();
        digitalWrite(_pin, LOW);
        rx_len = 0;
        rx_array_inx = 0;
        return true;
		}
		
	  else{
	  //failed checksum, need to clear this out anyway
		rx_len = 0;
		rx_array_inx = 0;
    ESP_LOGE("EasyTransfer", "CS Fail.");
		return false;
	  }
        
    }
  }
  
  return false;

//   uint8_t buffer[128] = {0};

//   if (_stream->available() < 3) {
//     return false;
//   }
//   delay(5);
  
//   uint8_t len = 0;
//   while(_stream -> available() && len < 127) {
//     buffer[len] = _stream -> read();
//     ESP_LOGI("EasyTransfer", "Got: %x", buffer[len]);
//     len++;
//   }
//   ESP_LOGI("EasyTransfer", "Searching start.");
//   uint8_t start_index = 0;
//   for (uint8_t i = 0; i < len; i++) {
//     if (buffer[i] == 0x06) {
//       start_index = i;
//       break;
//     }

//     if (i == len) return false;
//   }
//   ESP_LOGI("EasyTransfer", "Found start.");
//   if (buffer[start_index + 1] != device_id) return false;

//   if (buffer[start_index + 3] != 0x85) {
//     return false;
//   }

//   rx_len = buffer[start_index + 2];

// // l  1  2  3  4  5  6  7  8
//   // 06 04 85 01 00 01 00 04

//   //make sure the binary structs on both Arduinos are the same size.
//   if(rx_len != size){
//     rx_len = 0;
//     return false;
//   }

//   calc_CS = rx_len;
//   for (int i = start_index; i < rx_len; i++){
//     calc_CS^=buffer[i + start_index + 4];
//   } 
  
//   ESP_LOGI("EasyTransfer", "Verifying Checksum.");
//   if(calc_CS == buffer[start_index + rx_len + 4]){//CS good
//     ESP_LOGI("EasyTransfer", "CS good.");
//     memcpy(address,&buffer[start_index + 4], size);
//     rx_len = 0;
//     digitalWrite(_pin, HIGH);
//     delayMicroseconds(10);
//     _stream->write(0xFE); // Write ACK.
//     _stream->flush();
//     digitalWrite(_pin, LOW);
//     return true;
//   }

//   //failed checksum, need to clear this out anyway
//   rx_len = 0;
//   return false;
}

