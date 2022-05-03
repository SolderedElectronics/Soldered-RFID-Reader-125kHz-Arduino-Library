#ifndef __ATTINY_RFID_H__
#define __ATTINY_RFID_H__

#include "Arduino.h"

static volatile uint8_t _rfidRawData[48];

static volatile uint16_t _rfidRawDataCnt = 0;

class tinyRFID
{
  public:
    void begin();
    bool available();
    uint32_t getID();
    uint32_t getID(uint64_t _d);
    uint64_t getRAW();
    void clear();

    void makeTone(uint16_t _freq, uint16_t _dur = 0);
    void disableTone();

  private:
    const uint32_t RFID_HEADER = 0b101010101010101010;
    uint64_t _rfidRaw = 0;
    uint32_t _tagID = 0;

    void startTimer();
    void startRFOsc();
    bool validData(uint64_t _d);
    bool checkParity(uint16_t _d, uint8_t _n);
    bool checkParityR(uint64_t _d);
    bool checkParityC(uint64_t _d);
    bool checkStopBit(uint64_t _d);
};

#endif