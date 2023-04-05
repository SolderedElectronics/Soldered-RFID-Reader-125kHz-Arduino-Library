/**
 **************************************************
 *
 * @file        Generic-easyC-SOLDERED.cpp
 * @brief       RFID functions and methods.
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors   Borna Biro for soldered.com
 ***************************************************/


#include "RFID-SOLDERED.h"

/**
 * @brief                   Sensor specific native constructor.
 *
 * @param int _pin          Example parameter.
 */
Rfid::Rfid(int _rxPin, int _txPin, uint32_t _baud)
{
    rfidSerial = new SoftwareSerial(_rxPin, _txPin);
    rxPin = _rxPin;
    txPin = _txPin;
    baudRate = _baud;
    native = 1;
}

Rfid::Rfid()
{
    native = 0;
}

/**
 * @brief                   Initialization of the native mode (serial / UART communication with the RFID).
 */
void Rfid::initializeNative()
{
    rfidSerial->begin(baudRate);
}

bool Rfid::checkHW()
{
    if (native)
    {
        // Send a ping command and wait for UART to sent it.
        rfidSerial->println("#rfping");
        rfidSerial->flush();

        // Wait a little bit.
        delay(15);

        // Local serial buffer for storing the response.
        char _serailBuffer[25];

        // Check if we got something.
        if (getTheSerialData(_serailBuffer, sizeof(_serailBuffer) / sizeof(char), 5))
        {
            // Check if we got proper response.
            if (strstr(_serailBuffer, "#hello"))
            {
                // If it is, return true.
                return true;
            }
        }
    }
    else
    {
        // Ping the module on it's default I2C address.
        Wire.beginTransmission(address);

        // Received ACK? Retrun success!
        if (!Wire.endTransmission())
        {
            return true;
        }
    }

    // Something is wrong, retrun false (no module detected).
    return false;
}

/**
 * @brief                   Check if there is new RFID data available.
 *
 * @return                  bool - True if available, false if not.
 */
bool Rfid::available()
{
    // Variable that holds the state of the of new tag being detected.
    // Set it to false, just in case.
    bool _availableFlag = false;

    if (native)
    {
        // Local serial buffer for storing the response.
        char _serailBuffer[25];

        // If the data is available and it's valid, return success.
        if (getTheSerialData(_serailBuffer, sizeof(_serailBuffer) / sizeof(char), SERIAL_TIMEOUT_MS))
        {
            // Try to get the RFID tag ID.
            char *_tagIdStart = strchr(_serailBuffer, '$');
            char *_tagRawStart = strchr(_serailBuffer, '&');
            if (_tagIdStart && _tagRawStart)
            {
                // Get the ID by converting it from string to the int.
                tagID = atol(_tagIdStart + 1);
                rfidRAW = getUint64(_tagRawStart + 1);

                // Check if the result is non-zero.
                if (tagID && rfidRAW)
                    _availableFlag = true;
            }
        }
    }
    else
    {
        // To check if there is new RFID data avaialble, set register address to 0.
        sendAddress(0);

        // Read the data (but first cast it to char*).
        readData((char *)(&_availableFlag), 1);
    }

    return _availableFlag;
}

/**
 * @brief                   Get the RFID Tag ID number.
 *
 * @return                  uint32_t - Returns 32 bit tag ID number and clear it after reading.
 */
uint32_t Rfid::getId()
{
    uint32_t _tagID;

    if (native)
    {
        // Copy the tag ID into local variable.
        _tagID = tagID;

        // Clear the tag ID stored in the class.
        tagID = 0;
    }
    else
    {
        // To read RFID tag ID, set register address to 1.
        sendAddress(1);

        // Read the data (but first cast it to char*). RFID data is 4 bytes.
        // Tag ID is automatically cleared in the breakout after reading it.
        readData((char *)(&_tagID), 4);
    }

    // Retrun the result.
    return _tagID;
}

uint64_t Rfid::getRaw()
{
    uint64_t _rfidRaw;

    if (native)
    {
        // Copy the RFID RAW data into local variable.
        _rfidRaw = rfidRAW;

        // Clear the RFID RAW data stored in the class.
        rfidRAW = 0;
    }
    else
    {
        // To read RFID RAW data, set register address to 2.
        sendAddress(2);

        // Read the data (but first cast it to char*). RFID RAW data is 8 bytes.
        // RFID RAW data, is automatically cleared in the breakout after reading it.
        readData((char *)(&_rfidRaw), 8);
    }

    // Retrun the result.
    return _rfidRaw;
}

bool Rfid::getTheSerialData(char *_data, int _n, int _serialTimeout)
{
    // Holds success of getting the serial data.
    bool _ret = false;

    // Capture the current millis state (needed for the timeout).
    unsigned long _timeout = millis();

    // Used for indexing the array.
    int n = 0;

    // If there is something in the serial buffer, try to read that data.
    if (rfidSerial->available())
    {
        // Read chars until you hit the timeout.
        while ((unsigned long)(millis() - _timeout) < _serialTimeout)
        {
            // If there is new data un the serial buffer, save if in the buffer.
            if (rfidSerial->available())
            {
                // Check if there is still memory available in the buffer. If not start "dropping" incoming data.
                if (n < (_n - 2))
                {
                    // Save it to the local buffer.
                    _data[n++] = rfidSerial->read();

                    // Update the timeout.
                    _timeout = millis();
                }
                else
                {
                    // Drop the incoming data.
                    rfidSerial->read();
                }
            }
        }
    }

    // Add a null-terminating char at the end of the array.
    _data[n] = '\0';

    // If the variable for the indexing the array is not zero, that means we got something.
    if (n)
    {
        return true;
    }

    // Otherwise, return false.
    return false;
}

int Rfid::hexToInt(char _c)
{
    if ((_c >= '0') && (_c <= '9'))
        return (_c - '0');
    if ((_c >= 'A') && (_c <= 'F'))
        return ((_c - 'A') + 10);

    return 0;
}

uint64_t Rfid::getUint64(char *_c)
{
    uint64_t result = 0;
    for (int i = 0; i < 16; i++)
    {
        result += (uint64_t)(get16Base(15 - i)) * hexToInt(_c[i]);
    }

    return result;
}

uint64_t Rfid::get16Base(int _exp)
{
    uint64_t _result = 1;

    if (_exp == 0)
        return 1;

    for (int i = 0; i < _exp; i++)
    {
        _result *= 16;
    }

    return _result;
}