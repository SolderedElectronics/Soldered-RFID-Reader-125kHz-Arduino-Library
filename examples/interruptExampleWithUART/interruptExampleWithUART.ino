/**
 **************************************************
 *
 * @file        interruptExampleWithUART.ino
 * @brief       Interrupt example that shows how to read the RFID Tag ID using Soldered RFID brekaout and Serial (UART)
 *communication. Connect the module with Dasduino board, connect RFID antenna to the breakout board, connect INT pin
 *from RFID board to the D2 on Dasduino Core, upload the code and open the serial monitor. As soon as you place the
 *125kHz RFID Tag (or card) near antenna, you should see Tag ID number.
 *
 *              Using interrupts can be useful to get faster readings. This works with the idea that we do not need to
 *check constantly the RFID board for new RFID data, but the RFID board will inform us when there is new data available
 *(by pulling INT pin high).
 *
 *              Default UART speed is 9600, but communication speed can be changed by changing the position of the DIP
 *switches on the breakout.
 *
 *              Switch    1     2     3
 *              9600      0     0     0
 *              2400      0     0     1
 *              4800      0     1     0
 *              19200     0     1     1
 *              38400     1     0     0
 *              57600     1     0     1
 *              115200    1     1     0
 *              230400    1     1     1
 *
 *              Also, do not forget to change the communication speed in the RFID library constructor.
 *
 *	product:    www.solde.red/333154 - 125kHz RFID board with UART
 *
 * @authors     Borna Biro for Soldered.com
 ***************************************************/

// Include brekaout specific library.
#include "RFID-SOLDERED.h"

// Change pins if needed.
// Connect TXD from breakout to D4 on Dasduino Core.
#define RX_PIN 4

// Connect RXD from breakout to D5 on Dasduino Core.
#define TX_PIN 5

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

// RFID library constructor. Set RX pin, TX pin and baud for RFID communicaton speed (software serial).
Rfid rfid(RX_PIN, TX_PIN, 9600);

void setup()
{
    // Initialize the serial communication via UART
    Serial.begin(115200);

    // Initialize RFID library in native mode.
    rfid.begin();

    // Check hardware connections to  the module.
    if (!rfid.checkHW())
    {
        // Send message to the serial.
        Serial.println("No module detected, check wiring and baud rate!");

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