//---------------------------------------------------------------------------

#pragma once
#include "calculate_mask.h"
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
          4,                        26,
    0,  2,  5,  7,  9,    16, 14, 18, 20, 22, 28,
    1,  3,  6,  8,            15, 17, 19, 21, 27,
               10, 11,    12, 13,

    24, // usr button on MCU
};
constexpr uint32_t BUTTON_PIN_MASK = CALCULATE_MASK(BUTTON_PINS);

#define BOOTSEL_BUTTON_INDEX 27

//
// Button indexes
//         0                       1
//  2   3   4   5   6   |   7   8   9  10  11  12
// 13  14  15   16  6   |   7  17  18  19  20  21
//                 22  23    |    24  25
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Picosteno","layout":[{"x":0,"y":0,"w":4,"h":1},{"x":8,"y":0,"w":4,"h":1},{"x":0,"y":1},{"x":1,"y":1},{"x":2,"y":1},{"x":3,"y":1},{"x":4,"y":1,"w":1,"h":2},{"x":7,"y":1,"w":1,"h":2},{"x":8,"y":1},{"x":9,"y":1},{"x":10,"y":1},{"x":11,"y":1},{"x":12,"y":1},{"x":0,"y":2},{"x":1,"y":2},{"x":2,"y":2},{"x":3,"y":2},{"x":8,"y":2},{"x":9,"y":2},{"x":10,"y":2},{"x":11,"y":2},{"x":12,"y":2},{"x":3,"y":4},{"x":4,"y":4},{"x":7,"y":4},{"x":8,"y":4},{"x":5.75,"y":1.75,"s":0.5},{"x":5.75,"y":0.25,"s":0.5}]})"

const size_t BUTTON_COUNT = 28;

const char *const MANUFACTURER_NAME = "Noll Electronics LLC";
const char *const PRODUCT_NAME = "Picosteno (Javelin)";
const int VENDOR_ID = 0xFEED;

//---------------------------------------------------------------------------
