// Pin for interrupt output
#define INT_PIN PIN_PA4

// Interrupt pulse time (high time) in milliseconds.
#define INT_PIN_PULSE_MS 10

// Pins always used for address (in this case UART baud rate settings)
#define ADDRESS_PIN1 PIN_PA5
#define ADDRESS_PIN2 PIN_PA6
#define ADDRESS_PIN3 PIN_PA7

#define HELLO_PACKET "#rfping"

// Baud rate selectable with switches.
const uint32_t baudRatesLUT[8] = {9600, 2400, 4800, 19200, 38400, 57600, 115200, 230400};
