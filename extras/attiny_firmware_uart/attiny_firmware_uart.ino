/**
 **************************************************

   @file        attiny_firmware_uart.ino
   @brief       Firmware code for ATtiny1604 based RFID breakout with UART communication.

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

// Include library for RFID on ATtiny
#include "attiny_rfid.h"

// Include all settings for the ATtiny Firmware
#include "defines.h"

// Object for ATTINY RFID Class
tinyRFID rfid;

void setup()
{
    // Initialize the whole breakout board (RFID, pins, UART, etc).
    initSystem();
}

void loop()
{
    // Check if vaild RFID tag is read.
    if (rfid.available())
    {
        // Check if the ID is valid.
        if (rfid.getID() != 0)
        {
            // Everything is ok? Send TagID and RAW RFID data to UART.
            // Send '$' first to recognize the response packet from the Tag ID packet.
            Serial.print('$');
            Serial.print(rfid.getID());
            Serial.print('&');
            printHex64(rfid.getRAW());
            Serial.println();

            // Send an interrupt pulse
            pulseINT(INT_PIN_PULSE_MS);
        }

        // Clear everything from RFID buffer.
        rfid.clear();
    }

    // Check if there is something in the serial.
    serialResponse();
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

    // Set baud rate pins to input with pull up enabled.
    pinMode(ADDRESS_PIN1, INPUT_PULLUP);
    pinMode(ADDRESS_PIN2, INPUT_PULLUP);
    pinMode(ADDRESS_PIN3, INPUT_PULLUP);

    // Get the Serial baud rate.
    uint32_t baudRate = getBaudRate();

    // Set the UART baud rate
    Serial.begin(baudRate);
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
 * @brief       Sets the baud rate according to the switch's position.
 *
 * @return      uint32_t _baud - Selected baud rate (9600, 4800, 115200, etc). All possible baud rates are defined in
 * defines.h.
 */
uint32_t getBaudRate()
{
    // Get the state of the switches.
    uint8_t _switches =
        !digitalRead(ADDRESS_PIN3) | (!digitalRead(ADDRESS_PIN2) << 1) | (!digitalRead(ADDRESS_PIN1) << 2);

    // Convert that into baud rate using the baud rate LUT (defined in defines.h).
    uint32_t _baud = baudRatesLUT[_switches];

    // Return the new baud rate.
    return _baud;
}

/**
 * @brief       Checks, reads, proceses and makes a response to the incoming UART data. For now it's only used for
 * pinging the breakout.
 */
void serialResponse()
{
    // Check if there is something in the buffer
    if (Serial.available())
    {
        // Capture the time for the timeout.
        unsigned long _rxTimeout = millis();

        // Buffer for the incoming serial data.
        char _uartBuffer[20];

        // Counter for received chars.
        char _rxDataCount = 0;

        // Get the chars frim UART. Wait 100ms from last received char.
        while ((unsigned long)(millis() - _rxTimeout) < 10)
        {
            if (Serial.available() && (_rxDataCount < (sizeof(_uartBuffer) - 2)))
            {
                // Store the chars and convert them to the lowercase.
                _uartBuffer[_rxDataCount++] = tolower(Serial.read());

                // Get new timeout.
                _rxTimeout = millis();
            }
        }

        // Add null-terminating char.
        _uartBuffer[_rxDataCount] = '\0';

        // Check if the data is correct.
        if (strstr(_uartBuffer, HELLO_PACKET))
        {
            // Send respone back.
            Serial.println("#hello");
        }
    }
}

/**
 * @brief       Prints out a 64 bit HEX integer (since Arduino Print class cannot print 64 bits).
 *
 * @param       uint64_t _number
 *              64 bit interger that needs to be printed.
 */
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