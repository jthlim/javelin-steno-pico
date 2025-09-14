//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 100
#define BOOTSEL_BUTTON_INDEX 28

#define JAVELIN_DEBOUNCE_MS 10
#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t COLUMN_PINS[] = {24, 23, 21, 20, 19, 6, 5, 4, 3, 2, 1};
constexpr uint8_t ROW_PINS[] = {25, 18, 17};

// clang-format off
constexpr int8_t KEY_MAP[3][16] = {
  {  0,  1,  2,  3,  4, /**/  5,  6,  7,  8,  9, 10 },
  { 11, 12, 13, 14, 15, /**/ 16, 17, 18, 19, 20, 21 },
  { -1, -1, 22, 23, 24, /**/ 25, 26, 27, -1, -1, -1 },
};
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"n":"Uni v4","s":[{"c":"ff000000","s":"f","p":[{"c":"m","p":[-0.05,1]},{"c":"rl","p":[0,-1.05]},{"c":"ra","p":[13.1,0,0.05]},{"c":"ra","p":[0,2.1,0.05]},{"c":"ra","p":[-3,0,0.05]},{"c":"ra","p":[0,2,0.2]},{"c":"ra","p":[-8.1,0,0.05]},{"c":"ra","p":[0,-2,0.05]},{"c":"ra","p":[-2,0,0.2]},{"c":"ra","p":[0,-1,0.05]},{"c":"c"}]}],"k":[{"x":0,"y":0},{"x":1,"y":0},{"x":2,"y":0},{"x":3,"y":0},{"x":4,"y":0},{"x":7,"y":0},{"x":8,"y":0},{"x":9,"y":0},{"x":10,"y":0},{"x":11,"y":0},{"x":12,"y":0},{"x":0,"y":1},{"x":1,"y":1},{"x":2,"y":1},{"x":3,"y":1},{"x":4,"y":1},{"x":7,"y":1},{"x":8,"y":1},{"x":9,"y":1},{"x":10,"y":1},{"x":11,"y":1},{"x":12,"y":1},{"x":2,"y":3},{"x":3,"y":3},{"x":4,"y":3},{"x":7,"y":3},{"x":8,"y":3},{"x":9,"y":3},{"x":12.5,"y":3.5,"s":0.5}]})"

const size_t BUTTON_COUNT = 29;

const char *const MANUFACTURER_NAME = "stenokeyboards";
const char *const PRODUCT_NAME = "The Uni (Javelin)";
const int VENDOR_ID = 0x9000;

//---------------------------------------------------------------------------
