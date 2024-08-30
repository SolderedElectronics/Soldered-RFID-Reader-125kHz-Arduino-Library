#ifndef __ATTINY_RFID_H__
#define __ATTINY_RFID_H__

#include "Arduino.h"

// How long to keep RAW RFID data in the buffers (in milliseconds).
#define BUFFER_CLEAR_TIMEOUT_MS 1000ULL

class tinyRFID
{
  public:
    void begin();
    bool available();
    uint32_t getID();
    uint32_t getID(uint64_t _d);
    uint64_t getRAW();
    void clear(bool _clearTagData = true);

  private:
    const uint32_t RFID_HEADER = 0b101010101010101010UL;
    const uint32_t RFID_HEADER_MASK = 0b111111111111111111UL;
    uint64_t _rfidRaw = 0;
    uint32_t _tagID = 0;
    unsigned long _lastDecodeTimestamp = 0;

    bool findHeader(uint8_t *_data, int _n, int *_offset, int _skip);
    bool extractData(uint8_t *_rawData, int _n, uint64_t *_newData, int _offset);
    void startTimer();
    void startRFOsc();
    bool validData(uint64_t _d);
    bool checkParity(uint16_t _d, uint8_t _n);
    bool checkParityR(uint64_t _d);
    bool checkParityC(uint64_t _d);
    bool checkStopBit(uint64_t _d);
};

#endif
