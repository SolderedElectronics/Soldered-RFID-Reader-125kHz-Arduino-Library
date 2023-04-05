// Pin for interrupt output
#define INT_PIN PIN_PA4

// Interrupt pulse time (high time) in milliseconds.
#define INT_PIN_PULSE_MS 10

// Pins used for I2C address.
#define ADDRESS_PIN1 PIN_PA5
#define ADDRESS_PIN2 PIN_PA6
#define ADDRESS_PIN3 PIN_PA7

struct i2CRegsStruct
{
    int8_t addressPointer;
    bool tagAvailable;
    uint32_t tagID;
    uint64_t tagIDRaw;
};