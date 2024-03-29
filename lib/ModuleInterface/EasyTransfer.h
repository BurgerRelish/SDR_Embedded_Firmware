/******************************************************************
*  EasyTransfer Arduino Library 
*		details and example sketch: 
*			http://www.billporter.info/easytransfer-arduino-library/
*
*		Brought to you by:
*              Bill Porter
*              www.billporter.info
*
*		See Readme for other info and version history
*	
*  
*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*
*This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License. 
*To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or
*send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
******************************************************************/
#ifndef EasyTransfer_h
#define EasyTransfer_h


//make it a little prettier on the front end. 
#define details(name) (byte*)&name,sizeof(name)

#define EASYTRANSFER_TIMEOUT_US 100000

//Not neccessary, but just in case. 
#if ARDUINO > 22
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Stream.h"
//#include <NewSoftSerial.h>
//#include <math.h>
//#include <stdio.h>
//#include <stdint.h>
// #include <avr/io.h>
// #include <HardwareSerial.h>

class EasyTransfer {
public:
EasyTransfer() : rx_buffer(NULL) {}
~EasyTransfer() {free(rx_buffer);}
void begin(uint8_t * ptr, uint8_t length, Stream *theStream, uint8_t id, uint8_t dir_pin);
//void begin(uint8_t *, uint8_t, NewSoftSerial *theSerial);
bool sendData(uint8_t target_id);
boolean receiveData();
private:
Stream *_stream;
//NewSoftSerial *_serial;
uint8_t _pin = 0;
uint8_t device_id = 0; // id of device.
uint8_t * address;  //address of struct
uint8_t size = 0;       //size of struct
uint8_t * rx_buffer = NULL; //address for temporary storage and parsing buffer
uint8_t rx_array_inx = 0;  //index for RX parsing buffer
uint8_t rx_len = 0;		//RX packet length according to the packet
uint8_t calc_CS = 0;	   //calculated Chacksum
};



#endif
