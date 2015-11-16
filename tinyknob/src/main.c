#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv.h"

#include "calibration.h"
#include "rotary.h"

// USB HID Setup
// -----------------------------------------------------------------------------

static uchar    reportBuffer[2];    /* buffer for HID reports */

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};
/* We use a simplifed keyboard report descriptor with hort 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

static void buildReport(uchar mod, uchar letter) {
    reportBuffer[0] = mod;
    reportBuffer[1] = letter;
}

static uchar idleRate; // in 4 ms units

uchar usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            buildReport(0, 0);
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    } else {
        /* no vendor specific requests implemented */
    }
	return 0;
}

// Main
// -----------------------------------------------------------------------------

// Time in ms to wait between each key
#define DEBOUNCE  10000
uchar keydown = 0; // Whether the key is currently pressed

// Status LED

#define LED_PIN 4

void blink() {
    PORTB |= (1 << LED_PIN);
    _delay_ms(200);
    PORTB &= ~(1 << LED_PIN);
    _delay_ms(200);
}

// Calibrate when ready
void usbEventResetReady(void) {
    setCalibration();
}

int main() {
    wdt_disable();
	
    readCalibration();

	DDRB |= (1 << LED_PIN); // LED_PIN as output
    blink();

    // Disconnection-reconnection-enumeration dance

    usbDeviceDisconnect();

	uchar i;
    for(i=0;i<60;i++){  /* 600 ms disconnect */
        wdt_reset();
        _delay_ms(15);
    }

    usbDeviceConnect();

    // Set up everything else

    rotaryEncoderInit();

    wdt_enable(WDTO_1S);

    usbInit();

    // Main loop
    while(1) {
        wdt_reset(); // keep the watchdog happy
        usbPoll();

        if (usbInterruptIsReady()) {

            uchar report_mod = 0;
            uchar report_letter = 0;

            // Slight delay between keys
            if (recovering) recovering--;
            
            // Key is down, "release" it
            else if (keydown) {
                buildReport(0, 0);
                usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
                keydown = 0;

                next = 0;
            }

            // Turned in some direction
            else if ((next == 1) || (next == -1)) {
                PORTB |= (1 << LED_PIN);

                report_letter = next > 0
                    ? 0x80  // Volume up
                    : 0x81; // Volume down

                if (report_letter) {
                    buildReport(report_mod, report_letter);
                    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
                    keydown = 1;
                    recovering = DEBOUNCE;
                }

            }
        }
    }
	
    return 0;
}
