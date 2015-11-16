#include "usbdrv.h"

#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT      (1<<2)
#define MOD_COMMAND_LEFT      (1<<3)
#define KEY_0        39
#define KEY_UP       82

uchar modFromLetter(uchar ch) {
    if (ch == 'A') {
        return MOD_CONTROL_LEFT;
    }
    return 0;
}

// convert character to keycode
uchar keyFromLetter(uchar ch) {

    if(ch >= '0' && ch <= '9') {
        return (ch == '0') ? 39 : 30+(ch-'1');
    }

    else if(ch >= 'a' && ch <= 'z') {
        return 4+(ch-'a');
    }

    else if(ch >= 'A' && ch <= 'Z') {
        return 4+(ch-'A');
    }

    else {
        switch(ch) {
        case '.':
            return 0x37;
        case '_':
            //keyboard_report.modifier = MOD_SHIFT_LEFT;
        case '-':
            return 0x2D;
        case '^':
            return 0x52;
        case ' ':
            return 0x2C;
        case '\t':
            return 0x2B;
        case '\n':
            return 0x28;
        }
    }

    return ch; //fallback
}


