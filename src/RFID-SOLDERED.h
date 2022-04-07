/**
 **************************************************
 *
 * @file        Slider-potentiometer-with-easyC-SOLDERED.h
 * @brief       Basic functions for breakout board with slider potentiometer with easyc
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Goran Juric for soldered.com
 ***************************************************/

#ifndef _easyC_
#define _easyC_

#include "libs/Generic-easyC/easyC.hpp"

class sliderPot : public EasyC

{

  public:
    uint64_t getRaw(void);
    uint32_t getID(void);
    uint8_t available(void);
    void clear(void);

  protected:
    void initializeNative(){};

  private:
    int pin;
    uint64_t value;
    char raw[8];
};

#endif // !_easyC_
