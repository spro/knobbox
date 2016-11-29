# [knobbox](http://iams.pro/projects/knobbox)

The knobbox is a USB connected knob in a box, designed to be both useful and easy to make.

* Box models and laser cutting templates: https://github.com/spro/knobbox-boxes
* Firmware and circuit for ATTiny85 HID version, **tinyknob**: https://github.com/spro/knobbox-tinyknob
* Firmware and circuit for ATMega328P version, **megaknob**: https://github.com/spro/knobbox-megaknob
* OS X driver for megaknob: https://github.com/spro/knobbox-driver-osx

## Design Goals

### Hardware

* Minimal number of parts
* Common, easy to locate parts
* Only through-hole soldering
* Consumer friendly interface (USB)

### Software

* Plug and play as USB Human Interface Device
* Driver with UI showing and saving device state
* Driver allowing mode configuration
* Gracefully degrade to HID if no driver is available

### Enclosure

* Digitally fabricated (laser cut) and ordered online
* Parametric design to make the maximum use of material sheets
* Minimal/no extra hardware requirements (screws etc.)
