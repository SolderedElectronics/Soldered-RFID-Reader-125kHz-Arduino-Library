/**
 **************************************************

   @file        Template for attiny_firmware
   @brief       Fill in sensor specific code.

                MCU: ATTINY404
                Arduino Core: MegaTiny Core (http://drazzy.com/package_drazzy.com_index.json)
                Arduino Core Version: 2.4.2

                !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
                CPU CLOCK: 10MHz Internal - Changing this will affect timings of PWM signal for RFID coil (125 kHz), timings for Manchester decoding using timer TCB and buzzer freq.
                millis() / micros(): RTC (no micros)
                !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!

   @authors     Borna Biro @ soldered.com
                Optimized for ATTINY404 by Goran Juric
 ***************************************************/

#include "attiny_rfid.h"

#define DEBOUNCE_MS 2500
#define TIMEOUT_MS 300
tinyRFID rfid;

uint32_t last_scan = 0;

void setup()
{
  Serial.begin(57600);
  rfid.begin();
  pinMode(PIN_PA4, OUTPUT);
  digitalWrite(PIN_PA4, LOW);
}

void loop()
{
  if (rfid.available())
  {
    if ((rfid.getID() != 0) && (last_scan + DEBOUNCE_MS < millis()))
    {
      last_scan = millis();
      Serial.print(rfid.getID());
      digitalWrite(PIN_PA4, HIGH);
      delay(5);
      digitalWrite(PIN_PA4, LOW);
    }
    rfid.clear();
  }
  delay(5);
}
