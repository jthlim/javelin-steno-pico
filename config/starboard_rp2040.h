//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 100

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 1
#define JAVELIN_RGB_PIN 23

#define JAVELIN_BUTTON_MATRIX 0
#define JAVELIN_BUTTON_PINS 1

// clang-format off
constexpr uint8_t BUTTON_PINS[] = {
    8,  7,  2,  1,  0,  5, /**/  22, 28, 27, 26, 18, 17,  // Top row
    9,  10, 3,  4,  6,     /**/      21, 20, 19, 15, 16,  // Middle row
               11, 12,     /**/      13, 14,              // Thumb row

    24, // usr button on MCU
};
constexpr uint32_t BUTTON_PIN_MASK = 0x1d7fffff;

#define BOOTSEL_BUTTON_INDEX 27

//
// Button indexes
//  0   1   2   3   4   5    |     6   7   8   9  10  11
// 12  13  14  15  16   5    |     6  17  18  19  20  21
//                 22  23    |    24  25
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Starboard","layout":[{"x":0.53,"y":0.1,"r":0.52},{"x":1.43,"y":0.62,"r":0.52},{"x":2.58,"y":0.7,"r":0.52},{"x":3.6,"y":1,"r":0.52},{"x":4.4,"y":1.7,"r":0.52},{"x":5.05,"y":2.18,"w":1,"h":2,"r":0.52},{"x":7.95,"y":2.18,"w":1,"h":2,"r":-0.52},{"x":8.6,"y":1.7,"r":-0.52},{"x":9.4,"y":1,"r":-0.52},{"x":10.42,"y":0.7,"r":-0.52},{"x":11.57,"y":0.62,"r":-0.52},{"x":12.47,"y":0.1,"r":-0.52},{"x":0,"y":1,"r":0.52},{"x":0.9,"y":1.52,"r":0.52},{"x":2.05,"y":1.6,"r":0.52},{"x":3.07,"y":1.9,"r":0.52},{"x":3.87,"y":2.6,"r":0.52},{"x":9.13,"y":2.6,"r":-0.52},{"x":9.93,"y":1.9,"r":-0.52},{"x":10.95,"y":1.6,"r":-0.52},{"x":12.1,"y":1.52,"r":-0.52},{"x":13,"y":1,"r":-0.52},{"x":3.9,"y":4,"r":0.79},{"x":4.64,"y":4.74,"r":0.79},{"x":8.36,"y":4.74,"r":-0.79},{"x":9.1,"y":4,"r":-0.79},{"x":6.75,"y":5.5,"s":0.5},{"x":6.75,"y":3,"s":0.5}]})"

const size_t BUTTON_COUNT = 28;

const char *const MANUFACTURER_NAME = "Andrew Hess";
const char *const PRODUCT_NAME = "Starboard (Javelin)";
const int VENDOR_ID = 0xFEED;
const int PRODUCT_ID = 0xABCD;

//---------------------------------------------------------------------------
