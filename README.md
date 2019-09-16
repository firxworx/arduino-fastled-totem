# Arduino FastLED Totem

This project is for a FastLED-powered festival totem that uses WS2812B "NeoPixels" and is implemented on an Arduino Nano with an ATMega168 microcontroller.

The project consists of 6x LED strips that are each 11-pixels in length. They are wired together and hot-glued vertically around the diameter of the end of a bamboo shaft in an up-down-up-down-up-down pattern.

A button is incorporated into the project that changes patterns when pressed. The button is implemented with an interrupt and incorporates a basic debounce. The button works reasonably well to change the patterns immediately following a button press.

The project features a number of patterns, including a modified version of Fire2012 (originally by Mark Kriegsman) and several examples mostly from the FastLED demo reel.

The function `compute_bottom_to_top_offset()` is used to compute the index of a given pixel on a given strip (of the 6) such that the pattern moves from bottom-to-top. This is used to replicate an effect on one strip across all of the strips.

## Hardware Description

The project is powered by a typical 5V USB battery charger. It directly powers the Arduino Nano via its 5V USB jack.

The pixels are powered via the Arduino's 5V pin. The Nano supports a max output of 500mA so the code limits the max power via the FastLED function `set_max_power_in_volts_and_milliamps()`.

* Pin D6 is connected to the data line of the pixels via a 470-ohm resistor
* Pin D2 is used for the button (D2 is a common interrupt pin across many Arduino variants)
* 5V is connected to the pixel positive + and used for the button
* Ground is connected to the pixel negative - and used for the button

The button is a momentary push button paired with a 10K-ohm resistor, attached to Pin D2, 5V and Ground to trigger HIGH when pushed.

A good future improvement would be to add another button to control the brightness of the pixels.
