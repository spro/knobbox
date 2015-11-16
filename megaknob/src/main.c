#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv.h"

uchar n_modes = 8;
uchar mode = 0;
uchar current = 0;
uchar max_value = 16;

#define R_PIN_0 0
#define G_PIN_0 1
#define B_PIN_0 2

#define R_PIN_1 7
#define G_PIN_1 6
#define B_PIN_1 5

#define R_ON_0 PORTB |= (1 << R_PIN_0);
#define R_ON_1 PORTD |= (1 << R_PIN_1);
#define R_OFF_0 PORTB &= ~(1 << R_PIN_0);
#define R_OFF_1 PORTD &= ~(1 << R_PIN_1);
#define G_ON_0 PORTB |= (1 << G_PIN_0);
#define G_ON_1 PORTD |= (1 << G_PIN_1);
#define G_OFF_0 PORTB &= ~(1 << G_PIN_0);
#define G_OFF_1 PORTD &= ~(1 << G_PIN_1);
#define B_ON_0 PORTB |= (1 << B_PIN_0);
#define B_ON_1 PORTD |= (1 << B_PIN_1);
#define B_OFF_0 PORTB &= ~(1 << B_PIN_0);
#define B_OFF_1 PORTD &= ~(1 << B_PIN_1);
#define R_ON R_ON_0 R_ON_1
#define G_ON G_ON_0 G_ON_1
#define B_ON B_ON_0 B_ON_1
#define R_OFF R_OFF_0 R_OFF_1
#define G_OFF G_OFF_0 G_OFF_1
#define B_OFF B_OFF_0 B_OFF_1

void color_off() {
    R_OFF
    G_OFF
    B_OFF
}

void color_red() {
    R_ON
    G_OFF
    B_OFF
}

void color_green() {
    R_OFF
    G_ON
    B_OFF
}

void color_blue() {
    R_OFF
    G_OFF
    B_ON
}

void color_yellow() {
    R_ON
    G_ON
    B_OFF
}

void color_aqua() {
    R_OFF
    G_ON
    B_ON
}

void color_purple() {
    R_ON
    G_OFF
    B_ON
}

void color_white() {
    R_ON
    G_ON
    B_ON
}

// USB Setup
// -----------------------------------------------------------------------------

#define USB_MODE_SET 0
#define USB_MODE_INC 1
#define USB_MODE_DEC 2
#define USB_VALUE_SET 3
#define USB_VALUE_INC 4
#define USB_VALUE_DEC 5
#define USB_READ 6
#define USB_RESET 7

static uchar reportBuf[16];
static uchar replyBuf[16] = "Hello there";

uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    switch(rq->bRequest) {
        case USB_MODE_SET:
            // TODO: Get mode from data
            mode = rq->wValue.bytes[0];
            return 0;
        case USB_MODE_INC:
            mode = (mode + 1) % n_modes;
            return 0;
        case USB_MODE_DEC:
            mode = (mode - 1) % n_modes;
            return 0;
        case USB_VALUE_SET:
            current = rq->wValue.bytes[0];
            return 0;
        case USB_VALUE_INC:
            //current = (current + 1) % max_value;
            if (current < max_value) current++;
            return 0;
        case USB_VALUE_DEC:
            //current = (current - 1) % max_value;
            if (current > 0) current--;
            return 0;
        case USB_READ:
            usbMsgPtr = replyBuf;
            return sizeof(replyBuf);
        case USB_RESET:
            // TODO: Reset somehow
            return 0;
        default:
            return USB_NO_MSG;
    }

	return 0;
}

uchar usbFunctionRead(uchar *data, uchar len) {
    data[0] = 0xf3;
    return len;
}

void usbFunctionWriteOut(uchar * data, uchar len) {
    color_red();
}

void usbEventResetReady(void) {
}

#define BLINK_MS 100

void blink() {
    PORTB |= (1 << B_PIN_0);
    _delay_ms(BLINK_MS);
    PORTB &= ~(1 << B_PIN_0);
    _delay_ms(BLINK_MS);
}

#define BUTTON_PIN 3
#define ROT_PIN_0 4
#define ROT_PIN_1 5

uint32_t recovering = 0;
#define DEBOUNCE   10000
int next = 0;

// MODES
//
// 0 = Volume
// 1 = Brightness
void show_mode() {
    switch (mode) {
        case 1: color_red(); return;
        case 2: color_green(); return;
        case 3: color_blue(); return;
        case 4: color_yellow(); return;
        case 5: color_aqua(); return;
        case 6: color_purple(); return;
        case 7: color_white(); return;
        default: color_off(); return;
    }
}

void show_next() {
    if (next == 1) {
        color_green();
    } else if (next == -1) {
        color_red();
    } else {
        color_off();
    }
}

int main() {
	DDRB |= (1 << R_PIN_0) | (1 << G_PIN_0) | (1 << B_PIN_0);
	DDRD |= (1 << R_PIN_1) | (1 << G_PIN_1) | (1 << B_PIN_1);

    blink();

    wdt_enable(WDTO_1S);
    usbInit();

    // Reconnection cycle
    usbDeviceDisconnect();
    uchar i;
    for(i=0;i<250;i++){  /* 500 ms disconnect */
        wdt_reset();
        _delay_ms(2);
    }
    usbDeviceConnect();

    PCMSK0 |= (1<<PCINT3) | (1<<PCINT4) | (1 << PCINT5);
    PCICR |= (1<<PCIE0); // Enable PCINT

    sei();

    PORTB |= (1 << BUTTON_PIN);
    PORTB |= (1 << ROT_PIN_0);
    PORTB |= (1 << ROT_PIN_1);

    //PORTB |= (1 << 4);
    //PORTB &= ~(1 << 3);

    blink();
    blink();

    uchar keydown = 0;

    int trying = 0;

    while(1) {
        wdt_reset(); // keep the watchdog happy
        usbPoll();
        show_mode();
        if (usbInterruptIsReady()) {
            reportBuf[0] = mode;
            reportBuf[1] = current;
            reportBuf[2] = next;
            usbSetInterrupt(reportBuf, 8);
            next = 0;
        }
    }

    return 0;
}

int read_encoding(int now, int last) {
    int diff = 0;

    if      ((last == 0b11) && (now == 0b01)) diff = -1;
    else if ((last == 0b01) && (now == 0b00)) diff = -1;
    else if ((last == 0b00) && (now == 0b10)) diff = -1;
    else if ((last == 0b10) && (now == 0b11)) diff = -1;

    else if ((last == 0b11) && (now == 0b10)) diff = 1;
    else if ((last == 0b10) && (now == 0b00)) diff = 1;
    else if ((last == 0b00) && (now == 0b01)) diff = 1;
    else if ((last == 0b01) && (now == 0b11)) diff = 1;

    /* if      ((last == 0b11) && (now == 0b01)) diff = -1; */
    /* else if ((last == 0b01) && (now == 0b00)) diff = -1; */
    /* else if ((last == 0b00) && (now == 0b10)) diff = -1; */
    /* else if ((last == 0b10) && (now == 0b11)) diff = -1; */

    /* else if ((last == 0b11) && (now == 0b10)) diff = 1; */
    /* else if ((last == 0b10) && (now == 0b00)) diff = 1; */
    /* else if ((last == 0b00) && (now == 0b01)) diff = 1; */
    /* else if ((last == 0b01) && (now == 0b11)) diff = 1; */

    return diff;
}

int counting = 0;

void show_diff(int diff) {
    /* next = diff; */
    /* return; */

    /* if (diff == 1) { */
    /*     color_green(); */
    /* } else if (diff == -1) { */
    /*     color_red(); */
    /* } else { */
    /*     color_off(); */
    /* } */

    if (recovering) return;

    counting += diff;
    /* if (abs(counting) == 3) { */
    /*     mode += diff; */
    /*     if ((mode < 0) || (mode > 7)) mode = 0; */
    /*     counting = 0; */
    /* } */
    /* return; */

    if (counting == 4) {
        next = 1;
        if (current < max_value)
            current = (current + next);
        counting = 0;
    }
    else if (counting == -4) {
        next = -1;
        if (current > 0)
            current = (current + next);
        counting = 0;
    }
}

int last = 0;
ISR(PCINT0_vect) {
    pressed();

    //rotated();
}

void pressed() {
    int pressed = (PINB & (1 << BUTTON_PIN)) == 0;
    if (pressed) {
        mode = (mode + 1) % n_modes;
    } else {
        rotated();
    }
}

void rotated() {
    int now = (PINB & ((1 << ROT_PIN_0) | (1 << ROT_PIN_1))) >> ROT_PIN_0;
    //show_encoded(now);

    int diff = read_encoding(now, last);
    show_diff(diff);

    last = now;
}

