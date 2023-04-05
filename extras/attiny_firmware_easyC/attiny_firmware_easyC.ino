/**
 **************************************************

   @file        attiny_firmware_easyC.ino
   @brief       Firmware code for ATtiny1604 based RFID breakout with easyC communication.

                MCU: ATTINY1604
                Arduino Core: MegaTiny Core (http://drazzy.com/package_drazzy.com_index.json)
                Arduino Core Version: 2.4.2
                OR
                Dasduino ATtiny Boards -> easyC Boards
                MCU: ATTINY1604

                !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!
                CPU CLOCK: 10MHz Internal - Changing this will affect timings of PWM signal for RFID coil (125 kHz),
 timings for Manchester decoding using timer TCB and buzzer freq. millis() / micros(): RTC (no micros)
                !!!IMPORTANT!!! - choose this options - !!!IMPORTANT!!!

   @authors     Borna Biro for soldered.com
 ***************************************************/

// Include the library for the Wire / I2C library.
#include <Wire.h>

// Include library for RFID on ATtiny
#include "attiny_rfid.h"

// Include all settings for the ATtiny Firmware
#include "defines.h"

// Include header file for the easyC stuff.
#include "easyC.h"

// Object for ATTINY RFID Class
tinyRFID rfid;

// Struct for I2C virtual registers
i2CRegsStruct i2cRegs;

void setup()
{
    // Initialize the whole breakout board (RFID, pins, etc).
    initSystem();

    // Initialize easyC (I2C) pins.
    initDefault();

    // Get the I2C address and initialize Wire library (easyC) with this address.
    Wire.begin(getI2CAddress());

    // Set I2C callbacks.
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}

void loop()
{
    // Check if vaild RFID tag is read.
    if (rfid.available())
    {
        // Check if the ID is valid.
        if (rfid.getID() != 0)
        {
            // If everything went ok, save the data.
            i2cRegs.tagAvailable = true;
            i2cRegs.tagID = rfid.getID();
            i2cRegs.tagIDRaw = rfid.getRAW();

            // Send an interrupt pulse
            pulseINT(INT_PIN_PULSE_MS);
        }

        // Clear everything from RFID buffer.
        rfid.clear();
    }
}

/**
 * @brief       Initializes system including RFID library, GPIO pins and serial communication.
 */
void initSystem()
{
    // Initialize RFID library
    rfid.begin();

    // Set interrupt pin as output
    pinMode(INT_PIN, OUTPUT);

    // Set it to low
    digitalWrite(INT_PIN, LOW);

    // Initialize struct for I2C communication (virtual I2C registers).
    memset(&i2cRegs, 0, sizeof(i2cRegs));

    // Set address pointer to -1 (no address is selected)
    i2cRegs.addressPointer = -1;
}

/**
 * @brief       Creates a Interrupt pulse on specific pin with specific logic high time.
 *
 * @param       uint8_t _pulseDelay
 *              Interrupt pulse logic high time in milliseconds.
 */
void pulseINT(uint16_t _pulseDelay)
{
    // Set interrupt pin to high
    digitalWrite(INT_PIN, HIGH);

    // Wait a little bit
    delay(_pulseDelay);

    // Set interrupt pin to low.
    digitalWrite(INT_PIN, LOW);
}

/**
 * @brief       Callback function for I2C communication Receiveing the data from the master.
 * 
 * @param       int howMany - number of bytes received from the master.
 */
void receiveEvent(int howMany)
{
    while (1 < Wire.available())
    {
        char c = Wire.read();
    }

    // Get the address pointer value.
    i2cRegs.addressPointer = Wire.read();

    // Check the boundaries.
    if (i2cRegs.addressPointer > 2) i2cRegs.addressPointer = 0;
}

/**
 * @brief       Callback function for I2C communication for requesting the data from ATtiny.
 *
 */
void requestEvent()
{
    // Send the data to the master accordingly to the pointer address.

    // Address 0 is available flag.
    if (i2cRegs.addressPointer == 0)
    {
        // Send available flag.
        Wire.write((uint8_t*)&i2cRegs.tagAvailable, sizeof(i2cRegs.tagAvailable));
    }
    // Address 1 is for RFID Tag ID
    else if (i2cRegs.addressPointer == 1)
    {
        // Send RFID Tag ID Data.
        Wire.write((uint8_t*)&i2cRegs.tagID, sizeof(i2cRegs.tagID));

        // Wait until data is sent.
        while (Wire.available());

        // Clear the flag and RFID Tag ID.
        i2cRegs.tagAvailable = false;
        i2cRegs.tagID = 0;
    }
    // Address 2 is for RFID Tag RAW Data.
    else if (i2cRegs.addressPointer == 2)
    {
        // Send RFID Tag RAW Data.
        Wire.write((uint8_t*)&i2cRegs.tagIDRaw, sizeof(i2cRegs.tagIDRaw));

        // Wait until data is sent.
        while (Wire.available());

        // Clear the flag and RFID Tag RAW Data.
        i2cRegs.tagAvailable = false;
        i2cRegs.tagIDRaw = 0;
    }
}
