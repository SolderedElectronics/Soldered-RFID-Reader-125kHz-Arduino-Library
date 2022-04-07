#include "attiny_rfid.h"

bool state = 1;
 

void tinyRFID::begin()
{
	pinMode(PIN_PA1, INPUT_PULLUP); // RFID Input
	pinMode(PIN_PB0, OUTPUT); // Buzzer
	pinMode(PIN_PA3, OUTPUT); // 125 kHz PWM for RFID coil
	pinMode(PIN_PA7, OUTPUT); // Relay
	startRFOsc();	// Create 125 kHz PWM for RFID coil with TCA Timer
	startTimer();	// Setup TCB timer for manchester decoding
	PORTA.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;	// Enable pin interrupts on PIN_PA1
  EasyC::begin();
}

bool tinyRFID::available()
{
	bool _ret = false;
	uint64_t _RFIDData = 0;
	
    // If there is more than 380 data bits stored in array, that means the array is full, so start analyzing data
	if (_rfidRawDataCnt >= 380)
	{
		// Disable interrupts during decoding (save CPU cycles and there is no chance of data corruption from interrutps)
		PORTA.PIN1CTRL &= ~(PORT_ISC_BOTHEDGES_gc);
		PORTA.INTFLAGS |= (1 << 1);

    bool flag_found = 0;
    int16_t rawDataStartIndex = 0;
    uint32_t tempArr = 0;
    for(uint16_t i = 0; i <  32; i++)
    {
      tempArr = 0;
      for(uint8_t j = 0; j < 8; j++)
      {
        tempArr = ((_rfidRawData[i] | ((uint32_t)_rfidRawData[i + 1] << 8) | ((uint32_t)_rfidRawData[i + 2] << 16 ) | ((uint32_t)_rfidRawData[i + 3] << 24 ) ) & (0b111111111111111111 << j)) >> j;       
        if(tempArr == RFID_HEADER)
        {
          flag_found = 1;
          break;
        }
        rawDataStartIndex++;
      }
      if(flag_found)
      {
        _RFIDData = 0;
        rawDataStartIndex;

        for(int j = 0; j < 128; j = j + 2)
        {
          if(!((_rfidRawData[(rawDataStartIndex + j) / 8]) & (1 << ((rawDataStartIndex + j) % 8))) &&
          (_rfidRawData[(rawDataStartIndex + j + 1) / 8]) & (1 << ((rawDataStartIndex + j + 1) % 8)))
          {

            _RFIDData |= 1ULL << (63 - j/2);
          }
        }
        break;
      }
      else
      {
        _RFIDData = 0;
        _ret = false;
      }
    }
		if(validData(_RFIDData) && flag_found)
		{
			_rfidRaw = _RFIDData;
			_tagID = getID(_RFIDData);
			_ret = true;
		}

    // After data analyzing clean buffer/array, reset data array counter and re-enable interrupt
    PORTA.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;
    memset(_rfidRawData, 0, 48 * sizeof(uint8_t));
    _rfidRawDataCnt = 0;
  }
  return _ret;
}

uint32_t tinyRFID::getID()
{
	return _tagID;
}

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

  uint32_t _id = 0;
  for (int i = 0; i < 8; i++)
  {
    _id |= ((_d >> (64 - 23 - (5 * i))) & 0x0f) << ((7 - i) * 4);
  }
  return _id;
}

uint64_t tinyRFID::getRAW()
{
	return _rfidRaw;
}

void tinyRFID::clear()
{
	_rfidRaw = 0;
	_tagID = 0;
	memset(_rfidRawData, 0, sizeof(_rfidRawData));
	_rfidRawDataCnt = 0;
}

void tinyRFID::makeTone(uint16_t _freq, uint16_t _timeout)
{
  if (_freq < 2451) return;
  uint8_t _per = 10E6 / (1 * _freq) - 1;
  uint8_t _cmp = _per / 2;
  TCA0.SPLIT.LPER = _per;
  TCA0.SPLIT.LCMP0 = _cmp;
  TCA0.SPLIT.CTRLB |= TCA_SPLIT_LCMP0EN_bm;

  if (_timeout != 0)
  {
    unsigned long _time = millis();
    while ((unsigned long)(millis() - _time) < _timeout);
    TCA0.SPLIT.CTRLB &= ~TCA_SPLIT_LCMP0EN_bm;
  }
}

void tinyRFID::disableTone()
{
  TCA0.SPLIT.CTRLB &= ~TCA_SPLIT_LCMP0EN_bm;
}

void tinyRFID::startRFOsc()
{
  TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm;
  TCA0.SPLIT.HPER = 4;
  TCA0.SPLIT.HCMP0 = 2;
  TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc | TCA_SPLIT_ENABLE_bm;
}

void tinyRFID::startTimer()
{
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCA_SINGLE_ENABLE_bm;
  TCB0.CTRLB = TCB_CNTMODE_INT_gc;
}

bool tinyRFID::validData(uint64_t _d)
{
  // Check if the stop bit is zero, check parity for each column and for each row
  if(_d )
  if (!checkStopBit(_d)) return 0;
  if (!checkParityC(_d)) return 0;
  if (!checkParityR(_d)) return 0;
  return 1;
}
 
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

bool tinyRFID::checkStopBit(uint64_t _d)
{
  return !(_d & 1);
}


// THIS IS NOT A FUNCTION, IT'S A INTERRUPT VECTOR (do not put that into doxygen!)
ISR(PORTA_PORT_vect)
{
  volatile uint16_t _time = TCB0.CNT;
  TCB0.CNT = 0;
  volatile uint8_t _state = (VPORTA.IN >> 1) & 1;
  volatile uint8_t _n = 0;
  // Get time interval for each change on pin (there will be two times, shorter means there is on symbol change, longer means symbol change has happend)
  if ((_time > 900) && (_time < 1900))
  {
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
  if (_rfidRawDataCnt < 380) _rfidRawDataCnt += _n;

  // Reset the counter and clear interrupt flags
  PORTA.INTFLAGS |= (1 << 1);
}