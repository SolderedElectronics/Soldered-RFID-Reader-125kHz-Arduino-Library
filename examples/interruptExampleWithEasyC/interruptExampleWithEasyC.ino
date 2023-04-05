/**
 **************************************************
 *
 * @file        interruptExampleWithEasyC.ino
 * @brief       Interrupt example that shows how to read the RFID Tag ID using Soldered RFID brekaout and easyC (I2C)
 *communication using interrupts. Connect the module with Dasduino board with easyC cable, connect RFID antenna to the
 *breakout board, connect INT pin from RFID to the D2 pin of the Dasduino Core board. upload the code and open the
 *serial monitor. As soon as you place the 125kHz RFID Tag (or card) near antenna, you should see Tag ID number.
 *
 *              Using interrupts can be useful to get faster readings. This works with the idea that we do not need to
 *check constantly the RFID board for new RFID data, but the RFID board will inform us when there is new data available
 *(by pulling INT pin high).
 *
 *              Default I2C address is 0x30 but communication speed can be changed by changing the position of the DIP
 *switches on the breakout.
 *
 *              Switch    1     2     3
 *              0x30      0     0     0
 *              0x31      0     0     1
 *              0x32      0     1     0
 *              0x33      0     1     1
 *              0x34      1     0     0
 *              0x35      1     0     1
 *              0x36      1     1     0
 *              0x37      1     1     1
 *
 *              Also, do not forget to change the address in setup!
 *
 *  products:   www.solde.red/333273 - 125kHz RFID board with easyC
 *              www.solde.red/108343 - easyC cable 10cm
 *
 * @authors     Borna Biro for Soldered.com
 ***************************************************/

// Include brekaout specific library.
#include "RFID-SOLDERED.h"

// RFID library constructor. For easyC usage, there should be no parameters sent to the constructor.
Rfid rfid;

// RFID INT pin is connected to the D2 of the Dasduino Core.
#define INT_PIN 2

// Flag for interrupt event. Set it to false at start up.
// Flag must be declared as volatile since it's used in the interrupt routine.
volatile bool rfidIntFlag = false;

// Interrupt function. It just sets the flag to true.
// NOTE: Do not read RFID from the interrupt function!
void isr()
{
    rfidIntFlag = true;
}

void setup()
{
    // Initialize the serial communication via UART
    Serial.begin(115200);

    // Initialize RFID library in easyC mode.
    // You can specify the I2C address if it's different from default one (0x30).
    // rifd.begin(0x32);
    rfid.begin();

    // Check hardware connections to  the module.
    if (!rfid.checkHW())
    {
        // Send message to the serial.
        Serial.println("No module detected, check wiring and I2C address!");

        // Stop the code
        while (1)
        {
            // For Dasduino Connect.
            delay(1);
        }
    }

    // Attach interrupt on INT pin of the RFID on rising edge.
    attachInterrupt(digitalPinToInterrupt(INT_PIN), isr, RISING);

    Serial.println("Place your tag near RFID antenna");
}

void loop()
{
    // Check if the interrupt flag is set. That means, there is new tag available.
    if (rfidIntFlag)
    {
        // Clear the flag, so we can be ready for the next RFID tag.
        rfidIntFlag = false;

        // Check if there is vaild tag data available.
        if (rfid.available())
        {
            // If there is, read it and print it on the serial.
            Serial.print("Tag available! Tag ID: ");
            Serial.print(rfid.getId());
            Serial.print(" RAW RFID Data: ");

            // Print out a RAW RFID data (with RFID header, RFID data, parity bits, etc).
            // Special function must be used in order to print 64 bit int.
            rfid.printHex64(rfid.getRaw());

            // Send a new line at the end.
            Serial.println();
        }
    }
}