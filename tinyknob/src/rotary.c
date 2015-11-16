#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rotary.h"

/* last = 0; // Last reading */
/* counting = 0; // Counting pulses, 3 pulses per notch */
/* next = 0; // Which turn direction to send next: 1=CW -1=CCW */
/* recovering = 0; */

// Rotary encoder pins

// Set up interrupts and pull up input pins
#define blnk PORTB |= (1 << 4); _delay_ms(90); PORTB &= ~(1 << 4); _delay_ms(90);

void rotaryEncoderInit() {

    PCMSK |= (1<<PCINT0) | (1<<PCINT1);
    GIMSK |= (1<<PCIE); // Enable PCINT

    PORTB |= (1 << ROT_PIN_0);
    PORTB |= (1 << ROT_PIN_1);

    _delay_ms(50);

    sei();
}

// Rotary encoder parsing
// -----------------------------------------------------------------------------

// Figure out the direction based on current and last reading
int read_encoding(int now, int last) {
    int diff = 0;

    if      ((last == 0b11) && (now == 0b01)) diff = 1;
    else if ((last == 0b01) && (now == 0b00)) diff = 1;
    else if ((last == 0b00) && (now == 0b10)) diff = 1;
    else if ((last == 0b10) && (now == 0b11)) diff = 1;

    else if ((last == 0b11) && (now == 0b10)) diff = -1;
    else if ((last == 0b10) && (now == 0b00)) diff = -1;
    else if ((last == 0b00) && (now == 0b01)) diff = -1;
    else if ((last == 0b01) && (now == 0b11)) diff = -1;

    return diff;
}

// Count the number of indents for this turn, consider a full notch at 3 pulses
void show_diff(int diff) {
    if (recovering) return;

    counting += diff;
    if (counting == 3) {
        next = 1;
        counting = 0;
    }
    if (counting == -3) {
        next = -1;
        counting = 0;
    }
}

// Pin change interrupt to handle rotary encoder change
ISR(PCINT0_vect) {
    int now = (PINB & ((1 << ROT_PIN_0) | (1 << ROT_PIN_1)));

    int diff = read_encoding(now, last);
    show_diff(diff);

    last = now;
}

