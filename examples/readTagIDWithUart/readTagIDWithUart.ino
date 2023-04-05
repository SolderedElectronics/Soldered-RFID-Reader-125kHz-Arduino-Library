/**
 **************************************************
 *
 * @file        readTagIDWithUart.ino
 * @brief       Simple example that shows hor to read the RFID Tag ID using Soldered RFID brekaout and Serial (UART)
 *communication. Connect the module with Dasduino board, connect RFID antenna to the breakout board, upload the code and
 *open the serial monitor. As soon as you place the 125kHz RFID Tag (or card) near antenna, you should see Tag ID
 *number.
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
 * 
 *	product: www.solde.red/333154
 *
 * @authors     Borna Biro for Soldered.com
 ***************************************************/

// Include brekaout specific library.
#include "RFID-SOLDERED.h"

// Change pins if needed.
// D4 on Dasduino Core.
#define RX_PIN 4

// D5 on Dasduino Core.
#define TX_PIN 5

// RFID library constructor. Set RX pin, TX pin and baud for sotware serial.
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

    Serial.println("Place your tag near RFID antenna");
}

void loop()
{
    // Check if there is any tag data available.
    if (rfid.available())
    {
        // If there is, read it and print it on the serial.
        Serial.print("Tag available! Tag ID: ");
        Serial.print(rfid.getId());
        Serial.print(" RAW RFID Data: ");
        printHex64(rfid.getRaw());
        Serial.println();
    }
}

void printHex64(uint64_t _number)
{
    char _temp[17];
    
    for (int i = 60; i >= 0; i-=4)
    {
        _temp[15 - (i / 4)] = intToHex((_number >> i) & 0x0F);
    }
    
    _temp[16] = '\0';
    
    Serial.print(_temp);
}

char intToHex(uint8_t _n)
{
    _n &= 0x0F;
    
    if (_n >= 0 && _n <= 9) return (_n + '0');
    if (_n >= 10 && _n <= 15) return (_n - 10) + 'A';
}