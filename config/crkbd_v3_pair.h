//---------------------------------------------------------------------------

#pragma once
#include "calculate_mask.h"
#include "pair_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 0
#define JAVELIN_USE_USER_DICTIONARY 0
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
#define JAVELIN_SPLIT_IS_MASTER 0
#define JAVELIN_SPLIT_IS_LEFT 0
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

constexpr uint8_t RIGHT_COLUMN_PINS[] = {20, 22, 26, 27, 28, 29};
constexpr uint32_t RIGHT_COLUMN_PIN_MASK = CALCULATE_MASK(RIGHT_COLUMN_PINS);
constexpr uint8_t RIGHT_ROW_PINS[] = {4, 5, 6, 7};
constexpr uint32_t RIGHT_ROW_PIN_MASK = CALCULATE_MASK(RIGHT_ROW_PINS);

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

const size_t BUTTON_COUNT = 42;

const char *const MANUFACTURER_NAME = "foostan";
const char *const PRODUCT_NAME = "Corne (Javelin)";
const int VENDOR_ID = 0x4653;
// const int PRODUCT_ID = 0x0002;

//---------------------------------------------------------------------------
