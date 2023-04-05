#ifndef __EASYC_FIRMWARE__
#define __EASYC_FIRMWARE__

// Pins used for address
#define ADDRESS_PIN1 PIN_PA5
#define ADDRESS_PIN2 PIN_PA6
#define ADDRESS_PIN3 PIN_PA7

// Default address used by easyC.
#define DEFAULT_ADDRESS 0x30

void initDefault()
{
    // Initialize default address pins.
    pinMode(ADDRESS_PIN1, INPUT_PULLUP);
    pinMode(ADDRESS_PIN2, INPUT_PULLUP);
    pinMode(ADDRESS_PIN3, INPUT_PULLUP);
}

char getI2CAddress()
{
    // Inverted values cause hardware is setup that way.
    char addrPin1 = !digitalRead(ADDRESS_PIN1);
    char addrPin2 = !digitalRead(ADDRESS_PIN2);
    char addrPin3 = !digitalRead(ADDRESS_PIN3);

    // Default address if all pins are on off position.
    char address = DEFAULT_ADDRESS;
    address |= (addrPin3 << 0) | (addrPin2 << 1) | (addrPin1 << 2);

    return address;
}

#endif