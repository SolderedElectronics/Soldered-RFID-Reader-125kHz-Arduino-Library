/**
 **************************************************
 *
 * @file        Slider-potentiometer-with-easyC-SOLDERED.cpp
 * @brief       Basic functions for breakout board with slider potentiometer with easyc
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Goran Juric for soldered.com
 ***************************************************/


#include "RFID-SOLDERED.h"

/**
 * @brief       Function for reading value of potentiometer reading
 *
 * @return      value of potentiometer reading
 */
uint64_t sliderPot::getRaw()
{
    memset(raw, 0, 8 * sizeof(uint8_t));
    readRegister(0x01, raw, 8 * sizeof(uint8_t));
    value = raw[0] | (raw[1] << 8) | (raw[2] << 16) | (raw[3] << 24) | (raw[4] << 32) | (raw[5] << 40) |
            (raw[6] << 48) | (raw[7] << 56);
    return value;
}

/**
 * @brief       Function for reading minimal value of potentiometer reading
 *
 * @return      minimal value of potentiometer reading
 */
uint32_t sliderPot::getID()
{
    memset(raw, 0, 8 * sizeof(uint8_t));
    readRegister(0x02, raw, 4 * sizeof(uint8_t));
    value = raw[0] | (raw[1] << 8) | (raw[2] << 16) | (raw[3] << 24);
    return value;
}

/**
 * @brief       Function for reading maximal value of potentiometer reading
 *
 * @return      maximal value of potentiometer reading
 */
uint8_t sliderPot::available()
{
    memset(raw, 0, 8 * sizeof(uint8_t));
    readRegister(0x00, raw, 1 * sizeof(uint8_t));
    return raw[0];
}

/**
 * @brief       Function for reading percent vaule of potentiometer reading
 *
 * @return      percent vaule of potentiometer reading
 */
void sliderPot::clear()
{
    memset(raw, 0, 8 * sizeof(uint8_t));
    raw[0] = 0x03;
    sendData((const uint8_t *)raw, 1 * sizeof(uint8_t));
}