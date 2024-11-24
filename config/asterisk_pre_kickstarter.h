//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 100
#define BOOTSEL_BUTTON_INDEX 26

#define JAVELIN_BUTTON_MATRIX 0
#define JAVELIN_BUTTON_TOUCH 1

// clang-format off
constexpr uint8_t BUTTON_TOUCH_PINS[26] = {
           0,         /**/           25,
   8,  6,  2,  3,  1, /**/ 24, 22, 23, 19, 17, 12,
   9,  7,  5,  4,     /**/     21, 20, 18, 16, 13,
              10, 11, /**/ 15, 14,
};
constexpr uint32_t BUTTON_TOUCH_PIN_MASK = 0x03ffffff;
constexpr float BUTTON_TOUCH_THRESHOLD = 1.20f;
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Asterisk","layout":[{"x":0,"y":-0.5,"w":4,"h":0.5},{"x":8,"y":-0.5,"w":4,"h":0.5},{"x":0,"y":0},{"x":1,"y":0},{"x":2,"y":0},{"x":3,"y":0},{"x":4,"y":0,"w":1,"h":2},{"x":7,"y":0,"w":1,"h":2},{"x":8,"y":0},{"x":9,"y":0},{"x":10,"y":0},{"x":11,"y":0},{"x":12,"y":0},{"x":0,"y":1},{"x":1,"y":1},{"x":2,"y":1},{"x":3,"y":1},{"x":8,"y":1},{"x":9,"y":1},{"x":10,"y":1},{"x":11,"y":1},{"x":12,"y":1},{"x":2.5,"y":3},{"x":3.5,"y":3},{"x":7.5,"y":3},{"x":8.5,"y":3},{"x":5.75,"y":2.5,"s":0.5}]})"

const size_t BUTTON_COUNT = 27;

const char *const MANUFACTURER_NAME = "stenokeyboards";
const char *const PRODUCT_NAME = "Asterisk (Javelin)";
const int VENDOR_ID = 0x9000;

//---------------------------------------------------------------------------
