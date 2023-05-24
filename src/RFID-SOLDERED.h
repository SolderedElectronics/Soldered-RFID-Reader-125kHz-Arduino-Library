/**
 **************************************************
 *
 * @file        RFID-SOLDERED.h
 * @brief       Header file for RFID breakout board.
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro for soldered.com
 ***************************************************/

#ifndef __RFID_BOARD__
#define __RFID_BOARD__

#include "Arduino.h"
#include "libs/Generic-easyC/easyC.hpp"

#if defined(ARDUINO_ESP32_DEV)
#include "libs/ESPSoftwareSerial/ESPSoftwareSerial.h"
#else
#include "SoftwareSerial.h"
#endif

// How long serial will still try to get the data from the last char that has been received.
#define SERIAL_TIMEOUT_MS 20

class Rfid : public EasyC
{
  public:
    Rfid();
    Rfid(int _rxPin, int _txPin, uint32_t _baud);
    bool checkHW();
    bool available();
    uint32_t getId();
    uint64_t getRaw();
    void printHex64(uint64_t _number);
    void clear();

  protected:
    void initializeNative();

  private:
    bool getTheSerialData(char *_data, int _n, int _serialTimeout);
    uint64_t getUint64(char *_c);
    int hexToInt(char _c);
    uint64_t get16Base(int _exp);
    char intToHex(uint8_t _n);

    // Software Serial UART pins.
    int rxPin;
    int txPin;

    // Software Serial baud rate. Default is 9600.
    int baudRate;

    // Software serial object (used for serial communication with RFID breakout board).
    SoftwareSerial *rfidSerial;

    // Variables that holds the tagID for the serial.
    uint32_t tagID = 0;

    // Variables that holds the RFID RAW data for the serial.
    uint64_t rfidRAW = 0;
};

#endif