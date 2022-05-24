/**
 **************************************************
 *
 * @file        Read_ID.ino
 * @brief       Example for reading tags ID
 *
 *
 *	product: www.solde.red/333131
 *
 * @authors     Goran Juric for Soldered.com
 ***************************************************/

#include <SoftwareSerial.h>

// Open another port for serial communication, but software based 
// because first port is used to communicate with PC
SoftwareSerial rfid(4, 5); // RX, TX

volatile bool flag = 0;

void ISR_RFID()
{
  flag = 1; // Set flag to 1
}

void setup()
{
    // Initialize the serial communication via UART
    Serial.begin(115200);

    // Initialize UART serial communication with RDIF
    rfid.begin(38400);

    // Attach interrupt to pin 2. When RFID sensor picks up tags ID,
    // it sends interrupt signal and then sends ID via UART
    // Set interrupt to pin 2 on falling edge and call function ISR_RFID
    attachInterrupt(digitalPinToInterrupt(2), ISR_RFID, FALLING);
}

// ISR should last as short as possible, so we set here flag to 1
// and wait in main program for flag to be 1

void loop()
{
  uint32_t ID = 0; // Make variable to store ID when it becomes available
  if(flag) // If flag is 1, try to read ID
  {
    if(rfid.available()) // If data is incoming via UART
    {
      Serial.print("Scanned tag ID: "); // Print info message
      ID = rfid.read(); // Read incoming ID
      Serial.println(ID); // Print ID
    }
    else
    {
      Serial.println("Unknown error occured"); // If ID cannot be read, print error message
    }
    flag = 0; // Clear flag when ID is read
  }
  delay(200); // Wait a bit
}
