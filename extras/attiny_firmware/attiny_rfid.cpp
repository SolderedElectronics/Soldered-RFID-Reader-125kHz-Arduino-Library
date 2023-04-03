#include "attiny_rfid.h"

// More samples means higher accuracy but longer detection.
#define RFID_SAMPLES 200

// Do not change (calculations for the buffer size).
#define CEILING(x, y)       (((x) + (y)-1) / (y))
#define RFID_RAW_ARRAY_SIZE CEILING(RFID_SAMPLES, 8)

// Buffer for the raw RFID data captured by pin change interrputs.
volatile uint8_t _rfidRawData[RFID_RAW_ARRAY_SIZE];
volatile uint16_t _rfidRawDataCnt = 0;


/**
 * @brief       Initializes RFID library (starts oscilator, timer, sets GPIO and Interrups).
 */
void tinyRFID::begin()
{
    // RFID input pin.
    pinMode(PIN_PA1, INPUT_PULLUP);

    // 125 kHz PWM output for RFID coil.
    pinMode(PIN_PA3, OUTPUT);

    // Create 125 kHz PWM for RFID coil with TCA Timer and send it to the GPIO pin.
    startRFOsc();

    // Setup TCB timer for manchester decoding.
    startTimer();

    // Enable pin interrupts on PIN_PA1.
    PORTA.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;
}

/**
 * @brief       Checks if new RFID tag is available.
 *
 * @return      bool _ret. True if there is new tag ID detected, false if not (or it's not a valid data).
 */
bool tinyRFID::available()
{
    // Set return value to false.
    bool _ret = false;

    // Variable for decoded RFID data.
    uint64_t _RFIDData = 0;

    // If there is more than 170 data bits stored in array, that means the array is full, so start analyzing data
    if (_rfidRawDataCnt >= (RFID_SAMPLES - 10))
    {
        // Disable interrupts during decoding (save CPU cycles and there is no chance of data corruption from
        // interrutps)
        PORTA.PIN1CTRL &= ~(PORT_ISC_BOTHEDGES_gc);
        PORTA.INTFLAGS |= (1 << 1);

        // Variables for storing the start position of the RFID data (in bits)
        int _startHeaderOffset = 0;

        // Stores if the header is found or not.
        bool _headerFound = false;

        // Counts how many data that look like a header are skipped.
        int _skippedHeaders = 0;

        // There can be multiple instances of data that can look like a header, but it isn't, so we must go multiple
        // times trough the array to find them all!
        do
        {
            // Try to find strt of the header.
            _headerFound = findHeader(_rfidRawData, sizeof(_rfidRawData), &_startHeaderOffset, _skippedHeaders);
            if (_headerFound)
            {
                // If the vaild header is found, try to get the data.
                if (extractData(_rfidRawData, sizeof(_rfidRawData), &_RFIDData, _startHeaderOffset))
                {
                    // Check if is valid data
                    if (validData(_RFIDData))
                    {
                        // If data is valid, save it!
                        _rfidRaw = _RFIDData;
                        _tagID = getID(_RFIDData);
                        _ret = true;
                    }
                }
            }

            // Increment the counter.
            _skippedHeaders++;

            // Repeat all that until you find a header but it's not proper RFID data.
        } while (_headerFound && !_ret);

        // After data analyzing clean buffer/array, reset data array counter and re-enable interrupt
        PORTA.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;
        memset(_rfidRawData, 0, RFID_RAW_ARRAY_SIZE * sizeof(uint8_t));
        _rfidRawDataCnt = 0;
    }
    return _ret;
}

/**
 * @brief       Function that tries to find a RFID header in non-altered RFID data stream.
 *
 * @param       uint8_t *_data
 *              Pointer to the data stream / RFID buffer.
 * @param       int _n
 *              Size of the buffer (in bytes)
 * @param       int *_offset
 *              Pointer to the variable where ofset from the start of the buffer to the start of header will be saved.
 * @param       int _skip
 *              In case of multiple "headers" (that may not be headers at all) how many detected headers needs to be
 * skipped.
 *
 * @return      bool _ret - True if header has been found, false if not.
 */
bool tinyRFID::findHeader(uint8_t *_data, int _n, int *_offset, int _skip)
{
    // Set the return flag to false (in the case the header is not found).
    bool _ret = false;

    // Variable that counts skipped headers (makred as "invalid")
    int _skipHeader;

    // Iterate though array. Header is long 18 BITS (NOT BYTES!).
    // Make one array with 64 bits and iteate trough them.
    // If no header is found, get the next element of the array
    // and do same thing again.
    for (int i = 0; i < (_n - 8); i++)
    {
        // Make 64 bit raw data.
        uint64_t temp = ((uint64_t)(_data[i])) | ((uint64_t)(_data[i + 1]) << 8) | ((uint64_t)(_data[i + 2]) << 16) |
                        ((uint64_t)(_data[i + 3]) << 24) | ((uint64_t)(_data[i + 4]) << 32) |
                        ((uint64_t)(_data[i + 5]) << 40) | ((uint64_t)(_data[i + 6]) << 48) |
                        ((uint64_t)(_data[i + 7]) << 56);

        // Iterate though all bits to find a RFID header.
        for (int j = 0; j < (64 - 18); j++)
        {
            // If the match has been found, save the start position of
            // the header and return success.
            if (((temp >> j) & RFID_HEADER_MASK) == RFID_HEADER && (_skipHeader == _skip))
            {
                // if (_i != NULL) *_i = i;
                // if (_j != NULL) *_j = j;
                if (_offset != NULL)
                    *_offset = ((i * 8) + j);
                return true;
            }
        }
    }

    // Header not found? Return fail.
    return false;
}

/**
 * @brief       Functions converts raw RFID data strem / buffer (with pin changes) to the RFID readable data (64 bits).
 *
 * @param       uint8_t *_rawData
 *              Pointer to the data stream / RFID buffer.
 * @param       int _n
 *              Size of the buffer (in bytes)
 * @param       uint64_t *_newData
 *              Pointer where decoded data needs to be saved.
 * @param       int *_offset
 *              Pointer to the variable where ofset from the start of the buffer to the start of header will be saved.
 *
 * @return      bool - True if the data converson / extraction if successful, false if not.
 */
bool tinyRFID::extractData(uint8_t *_rawData, int _n, uint64_t *_newData, int _offset)
{
    // Init the RFID data array to zero.
    *_newData = 0;

    // Check if data is out of bounaries.
    if ((_offset + 128) > (_n * 8))
        return false;

    // Extract the data!
    for (int i = 0; i < 128; i += 2)
    {
        // if (!(_rawData[(_offset + i) / 8] & ((_offset + i) % 8)) && (_rawData[(_offset + i + 1) / 8] & ((_offset + i
        // + 1) % 8)))
        if (!((_rawData[(_offset + i) / 8]) & (1 << ((_offset + i)) % 8)) &&
            ((_rawData[(_offset + i + 1) / 8]) & (1 << ((_offset + i + 1)) % 8)))
        {
            *_newData |= (1ULL) << (63 - (i / 2));
        }
    }

    // If everything went ok, return success.
    return true;
}

/**
 * @brief       Get last vaild RIFD Tag ID.
 *
 * @return      uint32_t _tagID - Retruns tagID in decimal form.
 */
uint32_t tinyRFID::getID()
{
    // Retrun last tag ID.
    return _tagID;
}

/**
 * @brief       Functions converts RFID data into RFID Tag ID.
 *
 * @param       uint64_t _d
 *              64 bit RFID decoded data (with RFID header, stop and parity bits).
 * 
 * @return      uint32_t _id - Returns RFID Tag ID in decimal form.
 */
uint32_t tinyRFID::getID(uint64_t _d)
{
    // Create RFID ID Code. RFID stream structure:
    /*
        1   1   1   1   1   1   1   1   1     (Header)
        ----------------------------------
                       D00 D01 D02 D03  P0    (8 bit version number or customer ID + parity bit)
                       D04 D05 D06 D07  P1
        ----------------------------------
                       D08 D09 D10 D11  P2    (32 Data Bits + parity bits at end of each 4 data bits)
                       D12 D13 D14 D15  P3
                       D16 D17 D18 D19  P4
                       D20 D21 D22 D23  P5
                       D24 D25 D26 D27  P6
                       D28 D29 D30 D31  P7
                       D32 D33 D34 D35  P8
                       D36 D37 D38 D39  P9
        ----------------------------------
                       PC0 PC1 PC2 PC3  S0    (4 Column Parity bits + stop bit)
    */

    // Initialize variable for RFID Tag ID (set it to zero).
    uint32_t _id = 0;
    for (int i = 0; i < 8; i++)
    {
        // Extract D08, D09, D10, D11, ... D38, D39 and combine them into one variable
        // Then conver them into DEC and there is our tagID.
        _id |= ((_d >> (64 - 23 - (5 * i))) & 0x0f) << ((7 - i) * 4);
    }
    return _id;
}

/**
 * @brief       Gets last vaild RFID Tag data (with header, stop and parity bits).
 *
 * @return      uint64_t _rfidRaw - Returns 64 bit RFID vaild data.
 */
uint64_t tinyRFID::getRAW()
{
    return _rfidRaw;
}

/**
 * @brief       Clears all previous data from RFID Tag (including all internal buffers). Must be used after vaild tag is
 * read.
 *
 */
void tinyRFID::clear()
{
    // Set the RAW data to zero.
    _rfidRaw = 0;

    // Set the tag ID to zero (invalid tag ID).
    _tagID = 0;

    // Clear the buffer for the raw RFID capture
    memset(_rfidRawData, 0, sizeof(_rfidRawData));

    // Set counter for the captured bits to zero.
    _rfidRawDataCnt = 0;
}

/**
 * @brief       Setup the timerA in split mode and send PWM signal to the pin (125kHz, 40% duty). Needed for the RFID
 * coil resonance circuit.
 *
 */
void tinyRFID::startRFOsc()
{
    // This is the timer used for renerating 125kHz, 40% duty cycle.
    // It uses TimerA in split mode.
    TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm;
    TCA0.SPLIT.HPER = 4;
    TCA0.SPLIT.HCMP0 = 1;
    TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc | TCA_SPLIT_ENABLE_bm;
}

/**
 * @brief       Sets up the timer for the RFID manchester decoding. It's using TimerB.
 *
 */
void tinyRFID::startTimer()
{
    // This timer is used for measuring time interval between two pin change events on RFID input.
    // It uses TimerB in counter mode (it does not uses interrupts).
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCA_SINGLE_ENABLE_bm;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;
}

/**
 * @brief       Functions check data validity by checking parity bits, stop bit and data itself (data must not be zero).
 *
 * @param       uint64_t _d
 *              64 bit RFID decoded data (with RFID header, stop and parity bits).
 * 
 * @return      bool - True if the data is vaild, false if not.
 */
bool tinyRFID::validData(uint64_t _d)
{
    // Check if the stop bit is zero, check parity for each column and for each row.
    // And also check if the data is not zero.
    if (!_d)
        return 0;

    // Check is the stop bit is valid.
    if (!checkStopBit(_d))
        return 0;

    // Check if the column parit is ok.
    if (!checkParityC(_d))
        return 0;

    // Check if the row parity is ok.
    if (!checkParityR(_d))
        return 0;

    // If every check went ok, retrun success.
    return 1;
}

/**
 * @brief       Functions check parity bits in the data stream.
 *
 * @param       uint16_t _d
 *              Data with parity bits at the end (LSB).
 * @param       uint8_t _n
 *              Sizeof the data part (in bits).
 * 
 * @return      bool - True if the data is vaild, false if not.
 */
bool tinyRFID::checkParity(uint16_t _d, uint8_t _n)
{
    uint8_t _p = _d & 1;
    uint8_t _q = 0;

    for (int i = _n; i > 0; i--)
    {
        _q ^= (_d >> i) & 1;
    }
    return !((_q ^ _p) & 1);
}

/**
 * @brief       Functions check row parity bits in decoded RFID data stream.
 *
 * @param       uint64_t _d
 *              64 bit RFID decoded data (with RFID header, stop and parity bits).
 * 
 * @return      bool - True if the data is vaild, false if not.
 */
bool tinyRFID::checkParityR(uint64_t _d)
{
    uint8_t offset = 64 - 9 - 5;
    uint16_t parityCheck = 0;
    uint8_t _parityData = 0;
    for (int i = 0; i < 10; i++)
    {
        _parityData = (_d >> offset) & 0b00011111;
        parityCheck |= checkParity(_parityData, 4) << i;
        offset -= 5;
    }
    return (parityCheck == 0b0000001111111111) ? true : false;
}

/**
 * @brief       Functions check column parity bits in decoded RFID data stream.
 *
 * @param       uint64_t _d
 *              64 bit RFID decoded data (with RFID header, stop and parity bits).
 * 
 * @return      bool - True if the data is vaild, false if not.
 */
bool tinyRFID::checkParityC(uint64_t _d)
{
    uint8_t parityCheck = 0;
    for (int i = 0; i < 4; i++)
    {
        uint16_t _parityData = 0;
        for (int j = 0; j < 11; j++)
        {
            uint8_t _offset = 54 - (j * 5) - i;
            _parityData |= ((_d >> _offset) & 1) << (10 - j);
        }
        parityCheck |= checkParity(_parityData, 10) << i;
    }
    return (parityCheck == 0b00001111) ? true : false;
}

/**
 * @brief       Functions checks stop bit.
 *
 * @param       uint64_t _d
 *              64 bit RFID decoded data (with RFID header, stop and parity bits).
 * 
 * @return      bool - True if the stop bit is vaild, false if not.
 */
bool tinyRFID::checkStopBit(uint64_t _d)
{
    return !(_d & 1);
}


// This is not a classic function, it's a interrupt vector (do not put that into doxygen!).
// It's called automatically when pin change occurs.
ISR(PORTA_PORT_vect)
{
    // Capture the time on timer.
    volatile uint16_t _time = TCB0.CNT;

    // Reset the timer counter.
    TCB0.CNT = 0;

    // Get the state of the pin.
    volatile uint8_t _state = (VPORTA.IN >> 1) & 1;

    // Variable that holds number of bits added in the array.
    volatile uint8_t _n = 0;

    // Get time interval for each change on pin (there will be two times, shorter means there is on symbol change,
    // longer means symbol change has happend)
    if ((_time > 900) && (_time < 1900))
    {
        // Add state of the
        _rfidRawData[_rfidRawDataCnt / 8] |= _state << ((_rfidRawDataCnt % 8));
        _n = 1;
    }
    else if ((_time > 2000) && (_time < 4000))
    {
        _rfidRawData[_rfidRawDataCnt / 8] |= _state << ((_rfidRawDataCnt % 8));
        _rfidRawData[(_rfidRawDataCnt + 1) / 8] |= _state << ((_rfidRawDataCnt + 1) % 8);
        _n = 2;
    }

    // If the buffer is full, stop incrementing counter (maybe better idea is to disable interrupts??)
    if (_rfidRawDataCnt < RFID_SAMPLES)
        _rfidRawDataCnt += _n;

    // Reset the counter and clear interrupt flags
    PORTA.INTFLAGS |= (1 << 1);
}
