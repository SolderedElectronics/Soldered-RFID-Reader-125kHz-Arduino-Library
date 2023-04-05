/**
 **************************************************
 *
 * @file        RFID-SOLDERED.cpp
 * @brief       RFID functions and methods.
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro for soldered.com
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
        char _serailBuffer[30];

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

/**
 * @brief                   Get the RFID RAW data with the headers, RAW Data, parity bits, etc.
 *
 * @return                  uint64_t - Returns 64 bit long RAW RFID data.
 */
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

    // Return the result.
    return _rfidRaw;
}

/**
 * @brief                  Prints out 64 bit number in HEX format in Serial.
 *
 * @param                  uint64_t _number
 *                         64 bit numer that will be printed in serial in HEX format.
 */
void Rfid::printHex64(uint64_t _number)
{
    // Temporary array for storing 64 bit HEX value.
    char _temp[17];

    // Convert 64 bit integer into HEX char array (since Arduino can't print 64 bit numbers).
    for (int i = 60; i >= 0; i -= 4)
    {
        // Go trough the whole 64 bit number, take 4 bits, convert them into char (HEX char), store them and move by
        // next 4 bits.
        _temp[15 - (i / 4)] = intToHex((_number >> i) & 0x0F);
    }

    // Add null-terminating char at the end of the string.
    _temp[16] = '\0';

    // Print hex int to the serial.
    Serial.print(_temp);
}

/**
 * @brief                   Function gets the data from the serial.
 *
 * @param                   char *_data
 *                          Ponter to the data buffer.
 * @param                   int _n
 *                          Size of the data buffer.
 * @param                   int _serialTimeout
 *                          Timeout from the last received char.
 *
 * @return                  bool - Return true if there is some data available, false if not.
 */
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

/**
 * @brief                   Converts HEX char array into 64 bit number (integer).
 *
 * @param                   char *_c
 *                          Char array that holds 64 bit HEX char array.
 *
 * @return                  uint64_t - 64 bit integer.
 */
uint64_t Rfid::getUint64(char *_c)
{
    // Initialize variable for the result (must be initialized to zero).
    uint64_t result = 0;

    // Convert every HEX char into integer (old school method converting HEX base to decimal base).
    for (int i = 0; i < 16; i++)
    {
        result += (uint64_t)(get16Base(15 - i)) * hexToInt(_c[i]);
    }

    // Return the result of the conversion.
    return result;
}

/**
 * @brief                   Converts HEX char into integer from 0 to 15.
 *
 * @param                   char *_c
 *                          Char that holds HEX char.
 *
 * @return                  int - Converted HEX char into integer (from 0 to 15).
 */
int Rfid::hexToInt(char _c)
{
    // Variable used to store return value.
    char _result = 0;

    // If the char is from 0 to 9 in ASCII, convert it to the number from 0 to 9.
    if ((_c >= '0') && (_c <= '9'))
        _result = _c - '0';

    // If the char is from A to F into ASCII, convert it to the number from 10 to 15.
    if ((_c >= 'A') && (_c <= 'F'))
        _result = (_c - 'A') + 10;

    // Return the result.
    return _result;
}

/**
 * @brief                   Calculates 16 base number for the given exponent.
 *
 * @param                   int _exp
 *                          Exponent (pow(16, _exp) alternative).
 *
 * @return                  uint64_t - Calculated value for the given exponent to the 16 base.
 */
uint64_t Rfid::get16Base(int _exp)
{
    // Variable that holds the result. Must be initialized to 1 (due to multiplication).
    uint64_t _result = 1;

    // If the exponent 0, return 1 (math).
    if (_exp == 0)
        return 1;

    // Otherwise, calculate it.
    for (int i = 0; i < _exp; i++)
    {
        _result *= 16;
    }

    // Return the result.
    return _result;
}

/**
 * @brief                   Converts integer (from 0 to 15) to HEX char.
 *
 * @param                   uint8_t _n
 *                          Number that will be converted into HEX char.
 *
 * @return                  char - Converted HEX char.
 */
char Rfid::intToHex(uint8_t _n)
{
    // Only numbers from 0 to 15 are allowed.
    _n &= 0x0F;

    // Variable used to store return value.
    char _result = 0;

    // If the number is between 0 and 9, convert it into ASCII number.
    if (_n >= 0 && _n <= 9)
        _result = _n + '0';

    // If the number is between 10 and 15, convert it into ASCII letter (from A to F).
    if (_n >= 10 && _n <= 15)
        _result = (_n - 10) + 'A';

    // Return the result.
    return _result;
}