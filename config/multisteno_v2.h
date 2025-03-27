//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 100

#define BOOTSEL_BUTTON_INDEX 42

#define JAVELIN_DEBOUNCE_MS 10
#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t COLUMN_PINS[] = {25, 24, 23, 22, 21, 20, 7, 4, 3, 2, 1, 0};
constexpr uint8_t ROW_PINS[] = {6, 5, 19, 18};

// clang-format off
constexpr int8_t KEY_MAP[4][16] = {
  {  0,  1,  2,  3,  4,  5, /**/  6,  7,  8,  9, 10, 11 },
  { 12, 13, 14, 15, 16, 17, /**/ 18, 19, 20, 21, 22, 23 },
  { 24, 25, 26, 27, 28, 29, /**/ 30, 31, 32, 33, 34, 35 },
  { -1, -1, -1, 36, 37, 38, /**/ 39, 40, 41, -1, -1, -1 },
};
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Multisteno","layout":[{"x":0,"y":0},{"x":1,"y":0},{"x":2,"y":0},{"x":3,"y":0},{"x":4,"y":0},{"x":5,"y":0},{"x":8,"y":0},{"x":9,"y":0},{"x":10,"y":0},{"x":11,"y":0},{"x":12,"y":0},{"x":13,"y":0},{"x":0,"y":1},{"x":1,"y":1},{"x":2,"y":1},{"x":3,"y":1},{"x":4,"y":1},{"x":5,"y":1},{"x":8,"y":1},{"x":9,"y":1},{"x":10,"y":1},{"x":11,"y":1},{"x":12,"y":1},{"x":13,"y":1},{"x":0,"y":2},{"x":1,"y":2},{"x":2,"y":2},{"x":3,"y":2},{"x":4,"y":2},{"x":5,"y":2},{"x":8,"y":2},{"x":9,"y":2},{"x":10,"y":2},{"x":11,"y":2},{"x":12,"y":2},{"x":13,"y":2},{"x":3.5,"y":4},{"x":4.5,"y":4},{"x":5.5,"y":4},{"x":7.5,"y":4},{"x":8.5,"y":4},{"x":9.5,"y":4},{"x":7,"y":-1,"s":0.5}]}
)"

const size_t BUTTON_COUNT = 43;

const char *const MANUFACTURER_NAME = "Noll Electronics LLC";
const char *const PRODUCT_NAME = "Multisteno (Javelin)";
const int VENDOR_ID = 0xFEED;

//---------------------------------------------------------------------------
