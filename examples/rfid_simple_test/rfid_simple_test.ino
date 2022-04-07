/*
 *  MCU: ATTINY1604
 *  Arduino Core: MegaTiny Core (http://drazzy.com/package_drazzy.com_index.json)
 *  Arduino Core Version: 2.4.2
 *  
 *  !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
 *  CPU CLOCK: 10MHz Internal - Changing this will affect timings of PWM signal for RFID coil (125 kHz), timings for Manchester decoding using timer TCB and buzzer freq.
 *  millis() / micros(): RTC (no micros)
 *  !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
 */
 
#include "attiny_rfid.h"

tinyRFID rfid;
 
void setup() {
  rfid.begin();
}

void loop() {
  if (rfid.available())
  {
    //Serial.print("RAW:");
    //printUint64(rfid.getRAW());
    //Serial.print("ID:");
    //Serial.println(rfid.getID(), DEC);
    rfid.clear();
  }
  //delay(2000);
}
