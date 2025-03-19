//---------------------------------------------------------------------------

#pragma once
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 500

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 54
#define JAVELIN_RGB_LEFT_COUNT 27
#define JAVELIN_RGB_PIN 0
#define JAVELIN_USE_RGB_MAP 1

#define JAVELIN_SPLIT 1
#define JAVELIN_SPLIT_PIO_CYCLES 20
#define JAVELIN_SPLIT_TX_PIN 1
#define JAVELIN_SPLIT_RX_PIN 1
#define JAVELIN_SPLIT_IS_MASTER 1
#define JAVELIN_SPLIT_IS_LEFT 1
// #define JAVELIN_SPLIT_SIDE_PIN xx
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 512

#define JAVELIN_DISPLAY_DRIVER 1306
#define JAVELIN_OLED_WIDTH 128
#define JAVELIN_OLED_HEIGHT 32
#define JAVELIN_DISPLAY_WIDTH 32
#define JAVELIN_DISPLAY_HEIGHT 128
#define JAVELIN_OLED_I2C i2c1
#define JAVELIN_OLED_SDA_PIN 2
#define JAVELIN_OLED_SCL_PIN 3
#define JAVELIN_OLED_I2C_ADDRESS 0x3c
#define JAVELIN_OLED_ROTATION 90

#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t LEFT_COLUMN_PINS[] = {29, 28, 27, 26, 22, 20};
constexpr uint32_t LEFT_COLUMN_PIN_MASK = 0x3c500000;
constexpr uint8_t LEFT_ROW_PINS[] = {4, 5, 6, 7};
constexpr uint32_t LEFT_ROW_PIN_MASK = 0x000000f0;

constexpr uint8_t RIGHT_COLUMN_PINS[] = {20, 22, 26, 27, 28, 29};
constexpr uint32_t RIGHT_COLUMN_PIN_MASK = 0x3c500000;
constexpr uint8_t RIGHT_ROW_PINS[] = {4, 5, 6, 7};
constexpr uint32_t RIGHT_ROW_PIN_MASK = 0x000000f0;

// clang-format off
//
// Button indexes
//  0   1   2   3   4   5    |     6   7   8   9  10  11
// 12  13  14  15  16  17    |    18  19  20  21  22  23
// 24  25  26  27  28  29    |    30  31  32  33  34  35
//               36  37  38  |  39  40  41
//
// RGB indices
//  Back:
//      2         1         0    |    27        28        29
//                               |
//      3         4         5    |    32        31        30
//                               |
//
//  Front:
//     24  23  18  17  10  09    |    36  37  44  45  50  51
//     25  22  19  16  11  08    |    35  38  43  46  49  52
//     26  21  20  15  12  07    |    34  39  42  47  48  53
//                   14  13  06  |  33  40  41

constexpr int8_t LEFT_KEY_MAP[4][6] = {
  {   0,  1,  2,  3,  4,  5, },
  {  12, 13, 14, 15, 16, 17, },
  {  24, 25, 26, 27, 28, 29, },
  {  -1, -1, -1, 36, 37, 38, },
};

constexpr int8_t RIGHT_KEY_MAP[4][6] = {
  {   6,  7,  8,  9, 10, 11, },
  {  18, 19, 20, 21, 22, 23, },
  {  30, 31, 32, 33, 34, 35, },
  {  39, 40, 41, -1, -1, -1, },
};

constexpr uint8_t RGB_MAP[54] = {
  24, 23, 18, 17, 10,  9,   36, 37, 44, 45, 50, 51,
  25, 22, 19, 16, 11,  8,   35, 38, 43, 46, 49, 52,
  26, 21, 20, 15, 12,  7,   34, 39, 42, 47, 48, 53,
              14, 13,  6,   33, 40, 41,

  // Underglow
  2, 1, 0, 27, 28, 29,
  3, 4, 5, 32, 31, 30,
};

// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Corne","layout":[{"x":0,"y":0.3},{"x":1,"y":0.3},{"x":2,"y":0.1},{"x":3,"y":0},{"x":4,"y":0.1},{"x":5,"y":0.2},{"x":9,"y":0.2},{"x":10,"y":0.1},{"x":11,"y":0},{"x":12,"y":0.1},{"x":13,"y":0.3},{"x":14,"y":0.3},{"x":0,"y":1.3},{"x":1,"y":1.3},{"x":2,"y":1.1},{"x":3,"y":1},{"x":4,"y":1.1},{"x":5,"y":1.2},{"x":9,"y":1.2},{"x":10,"y":1.1},{"x":11,"y":1},{"x":12,"y":1.1},{"x":13,"y":1.3},{"x":14,"y":1.3},{"x":0,"y":2.3},{"x":1,"y":2.3},{"x":2,"y":2.1},{"x":3,"y":2},{"x":4,"y":2.1},{"x":5,"y":2.2},{"x":9,"y":2.2},{"x":10,"y":2.1},{"x":11,"y":2},{"x":12,"y":2.1},{"x":13,"y":2.3},{"x":14,"y":2.3},{"x":3.5,"y":3.4},{"x":4.7,"y":3.58,"r":0.26},{"x":5.95,"y":3.6,"w":1,"h":1.5,"r":0.52},{"x":8.05,"y":3.6,"w":1,"h":1.5,"r":-0.52},{"x":9.3,"y":3.58,"r":-0.26},{"x":10.5,"y":3.4}]})"

const size_t BUTTON_COUNT = 42;

const char *const MANUFACTURER_NAME = "foostan";
const char *const PRODUCT_NAME = "Corne (Javelin)";
const int VENDOR_ID = 0x4653;
// const int PRODUCT_ID = 0x0001;

//---------------------------------------------------------------------------
