/**
 **************************************************
 *
 * @file        Template for attiny_firmware
 * @brief       Fill in sensor specific code.
 * 
 *              MCU: ATTINY404
 *              Arduino Core: MegaTiny Core (http://drazzy.com/package_drazzy.com_index.json)
 *              Arduino Core Version: 2.4.2
 *  
 *              !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
 *              CPU CLOCK: 10MHz Internal - Changing this will affect timings of PWM signal for RFID coil (125 kHz), timings for Manchester decoding using timer TCB and buzzer freq.
 *              millis() / micros(): RTC (no micros)
 *              !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
 *  
 * @authors     Borna Biro @ soldered.com
 *              Optimized for ATTINY404 by Goran Juric
 ***************************************************/

#include "easyC.h"
#include "attiny_rfid.h"
#include <Wire.h>

tinyRFID rfid;

char data_to_send = 0;

bool flag_available = 0;

int addr = DEFAULT_ADDRESS;

void setup()
{
    initDefault();
    addr = getI2CAddress();
    rfid.begin();
    Wire.begin(addr);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    pinMode(PIN_PA4, OUTPUT);
    digitalWrite(PIN_PA4,LOW);
}

void loop()
{
  if(!flag_available & rfid.available())
  {
      digitalWrite(PIN_PA4,HIGH);
      flag_available = 1;
      digitalWrite(PIN_PA4,LOW);
  }
  if(rfid.available() && rfid.getID() == 0)
  {
    flag_available = 0;
    rfid.clear();
  }
  
}

void receiveEvent(int howMany)
{
    while (1 <= Wire.available())
    {
        data_to_send = Wire.read();
    }
}

void requestEvent()
{
    char a[8];
    memset(a, 0, 8 * sizeof(char));
    uint64_t temp = 0;
    switch(data_to_send)
    {
      case 0:
        a[0] = (char)flag_available;
        Wire.write(a, 1);
        break;
      case 1:
        if(flag_available)
          temp = rfid.getRAW();
        else
          temp = 0;
        a[0] = temp & 0xFF;
        a[1] = (temp & 0xFF00) >> 8;
        a[2] = (temp & 0xFF0000) >> 16;
        a[3] = (temp & 0xFF000000) >> 24;
        a[4] = (temp & 0xFF00000000) >> 32;
        a[5] = (temp & 0xFF0000000000) >> 40;
        a[6] = (temp & 0xFF000000000000) >> 48;
        a[7] = (temp & 0xFF00000000000000) >> 56;
        Wire.write(a, 8);
        break;
      case 2:
        if(flag_available)
        {
          temp = rfid.getID();
          flag_available = 0;
          rfid.clear();
        }
        else
          //temp = 0;
        //temp = rfid.getID();
        a[0] = temp & 0xFF;
        a[1] = (temp & 0xFF00) >> 8;
        a[2] = (temp & 0xFF0000) >> 16;
        a[3] = (temp & 0xFF000000) >> 24;
        Wire.write(a, 4);
        break;
      case 3:
        rfid.clear();
        break;
    }
}
