//---------------------------------------------------------------------------

#pragma once
#include "encoder_pins.h"
#include "pair_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 0
#define JAVELIN_USE_USER_DICTIONARY 0
#define JAVELIN_USB_MILLIAMPS 500

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 62
#define JAVELIN_RGB_LEFT_COUNT 31
#define JAVELIN_RGB_PIN 0
#define JAVELIN_USE_RGB_MAP 1

#define JAVELIN_SPLIT 1
#define JAVELIN_SPLIT_PIO_CYCLES 16
#define JAVELIN_SPLIT_TX_PIN 1
#define JAVELIN_SPLIT_RX_PIN 1
#define JAVELIN_SPLIT_IS_MASTER 0
#define JAVELIN_SPLIT_SIDE_PIN 9
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 512

#define JAVELIN_DISPLAY_DRIVER 1306
#define JAVELIN_OLED_WIDTH 128
#define JAVELIN_OLED_HEIGHT 64
#define JAVELIN_DISPLAY_WIDTH 128
#define JAVELIN_DISPLAY_HEIGHT 64
#define JAVELIN_OLED_I2C i2c1
#define JAVELIN_OLED_TX_DMA I2C1_TX
#define JAVELIN_OLED_SDA_PIN 2
#define JAVELIN_OLED_SCL_PIN 3
#define JAVELIN_OLED_I2C_ADDRESS 0x3c
#define JAVELIN_OLED_ROTATION 180

#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t LEFT_COLUMN_PINS[] = {8, 27, 26, 22, 20, 23, 21};
constexpr uint8_t LEFT_ROW_PINS[] = {4, 5, 6, 7};

constexpr uint8_t RIGHT_COLUMN_PINS[] = {23, 4, 5, 6, 7, 8, 21};
constexpr uint8_t RIGHT_ROW_PINS[] = {27, 26, 22, 20};

// clang-format off
//
// Button indexes
//  0   1   2   3   4   5         |          6   7   8   9  10  11
// 12  13  14  15  16  17         |         18  19  20  21  22  23
// 24  25  26  27  28  29  30  31 | 32  33  34  35  36  37  38  39
//             40  41  42  43  44 | 45  46  47  48  49
//
// Matrix positions
// 06  05  04  03  02  01         |         01  02  03  04  05  06
// 16  15  14  13  12  11         |         11  12  13  14  15  16
// 26  25  24  23  22  21  33  20 | 20  33  21  22  23  24  25  26
//             34  32  31  35  30 | 30  35  31  32  34
//
// RGB indices
//  Back:
//      2         1         0          |          31        32        33
//                                     |
//      3         4         5          |          36        35        34
//                                     |
//
//  Front:
//     30  29  28  27  26  25          |          56  57  58  59  60  61
//     24  23  22  21  20  19          |          50  51  52  53  54  55
//     18  17  16  15  14  13  12  11  |  42  43  44  45  46  47  48  49
//                 10   9   8   7   6  |  37  38  39  40  41

constexpr int8_t LEFT_KEY_MAP[4][8] = {
  {  -1,  5,  4,  3,  2,  1,  0, -1 },
  {  -1, 17, 16, 15, 14, 13, 12, -1 },
  {  31, 29, 28, 27, 26, 25, 24, -1 },
  {  44, 42, 41, 30, 40, 43, -1, -1 },
};

constexpr int8_t RIGHT_KEY_MAP[4][8] = {
  {  -1,  6,  7,  8,  9, 10, 11, -1 },
  {  -1, 18, 19, 20, 21, 22, 23, -1 },
  {  32, 34, 35, 36, 37, 38, 39, -1 },
  {  45, 47, 48, 33, 49, 46, -1, -1 },
};

constexpr uint8_t RGB_MAP[62] = {
  30, 29, 28, 27, 26, 25,                   56, 57, 58, 59, 60, 61,
  24, 23, 22, 21, 20, 19,                   50, 51, 52, 53, 54, 55,
  18, 17, 16, 15, 14, 13, 12, 11,   42, 43, 44, 45, 46, 47, 48, 49,
              10,  9,  8,  7,  6,   37, 38, 39, 40, 41,

  // Underglow
  2, 1, 0, 31, 32, 33,
  3, 4, 5, 36, 35, 34,
};

// clang-format on

const size_t BUTTON_COUNT = 50;

#define JAVELIN_ENCODER 1
#define JAVELIN_ENCODER_COUNT 2
#define JAVELIN_ENCODER_LEFT_COUNT 1
#define JAVELIN_ENCODER_SPEED 2

constexpr EncoderPins ENCODER_PINS[] = {{29, 28}, {29, 28}};

const char *const MANUFACTURER_NAME = "splitkb";
const char *const PRODUCT_NAME = "Kyria (Javelin)";
const int VENDOR_ID = 0x8d1d;
// const int PRODUCT_ID = 0xcf44;

//---------------------------------------------------------------------------
