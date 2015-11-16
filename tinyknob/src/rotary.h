#define ROT_PIN_0 0
#define ROT_PIN_1 1

int last; // Last reading
int counting; // Counting pulses, 3 pulses per notch
int next; // Which turn direction to send next: 1=CW -1=CCW
uint32_t recovering; // Set to DEBOUNCE to start debouncing

void rotaryEncoderInit();
