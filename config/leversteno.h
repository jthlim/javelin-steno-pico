//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 100

#define JAVELIN_DEBOUNCE_MS 10
#define JAVELIN_BUTTON_MATRIX 0
#define JAVELIN_BUTTON_PINS 1

// clang-format off
constexpr uint8_t BUTTON_PINS[] = {
    21, // Number bar
    23, 20, 18, 15,  12,  10,  7,  5,  3,  1,
    22, 19, 17, 14,        9,  6,  4,  2,  0,
             16, 13,    11,  8
};

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"n":"Leversteno","k":[{"x":0,"y":0.3,"w":11,"h":0.6},{"x":0,"y":1,"w":0.8,"h":1.5},{"x":1,"y":1,"w":0.8,"h":1.5},{"x":2,"y":1,"w":0.8,"h":1.5},{"x":3,"y":1,"w":0.8,"h":1.5},{"x":4,"y":1,"w":1.8,"h":3.1},{"x":6,"y":1,"w":0.8,"h":1.5},{"x":7,"y":1,"w":0.8,"h":1.5},{"x":8,"y":1,"w":0.8,"h":1.5},{"x":9,"y":1,"w":0.8,"h":1.5},{"x":9.9,"y":1,"w":1.1,"h":1.5},{"x":0,"y":2.6,"w":0.8,"h":1.5},{"x":1,"y":2.6,"w":0.8,"h":1.5},{"x":2,"y":2.6,"w":0.8,"h":1.5},{"x":3,"y":2.6,"w":0.8,"h":1.5},{"x":6,"y":2.6,"w":0.8,"h":1.5},{"x":7,"y":2.6,"w":0.8,"h":1.5},{"x":8,"y":2.6,"w":0.8,"h":1.5},{"x":9,"y":2.6,"w":0.8,"h":1.5},{"x":9.9,"y":2.6,"w":1.1,"h":1.5},{"x":2.5,"y":4.2,"w":0.8,"h":1.5},{"x":3.5,"y":4.2,"w":0.8,"h":1.5},{"x":5.5,"y":4.2,"w":0.8,"h":1.5},{"x":6.5,"y":4.2,"w":0.8,"h":1.5}]})"

const size_t BUTTON_COUNT = 24;

const char *const MANUFACTURER_NAME = "Noll Electronics LLC";
const char *const PRODUCT_NAME = "Leversteno (Javelin)";
const int VENDOR_ID = 0x2e8a;
const int PRODUCT_ID = 0x1008;

//---------------------------------------------------------------------------
