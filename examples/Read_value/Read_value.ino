/**
 **************************************************
 *
 * @file        Read_value.ino
 * @brief       Example for using the digital and analog read functions for Slider potentiometer with easyC
 *
 *
 *	product: www.solde.red/333131
 *
 * @authors     Goran Juric for Soldered.com
 ***************************************************/

#include "RFID-SOLDERED.h"

// Declare the sensor object
sliderPot slider;

void setup()
{
    // Initialize the serial communication via UART
    Serial.begin(115200);

    // Initialize the sensor
    slider.begin();
}

void loop()
{
    if(slider.available())
    {
      Serial.print("Raw value: "); //Print information message
      uint64_t ttemp = slider.getRaw();
      Serial.print((uint32_t)(ttemp << 32), HEX); //Prints raw value of slider potentiometer
      Serial.println((uint32_t)ttemp, HEX); //Prints raw value of slider potentiometer
      
      Serial.print("ID: "); //Print information message
      Serial.println(slider.getID(),DEC); //Prints minimum value of potentiometer
    }
    else
    {
      Serial.println("No scanned tag.");
    }
    delay(1000);
    
}
