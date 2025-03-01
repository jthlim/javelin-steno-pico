//---------------------------------------------------------------------------

#pragma once
#include "encoder_pins.h"
#include "pair_flash_layout.h"

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 0
#define JAVELIN_USE_USER_DICTIONARY 0
#define JAVELIN_USB_MILLIAMPS 500

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 46
#define JAVELIN_RGB_LEFT_COUNT 23
#define JAVELIN_RGB_PIN 10
#define JAVELIN_USE_RGB_MAP 1

#define JAVELIN_SPLIT 1
#define JAVELIN_SPLIT_TX_PIN 12
#define JAVELIN_SPLIT_RX_PIN 12
#define JAVELIN_SPLIT_IS_MASTER 0
#define JAVELIN_SPLIT_IS_LEFT 0
// #define JAVELIN_SPLIT_SIDE_PIN xx
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 512

#define JAVELIN_BUTTON_MATRIX 0
#define JAVELIN_BUTTON_PINS 1

// clang-format off
constexpr uint8_t BUTTON_PINS[] = {
   0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, /**/ 25, 27,  1,  2,  3,  9,  8,
   0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, /**/ 23, 26, 28,  0,  4, 14, 11,
   0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,       /**/     22, 20, 29,  5, 18, 15,
                           0x7f, 0x7f, 0x7f, /**/ 19, 17, 16,
};
constexpr uint32_t BUTTON_PIN_MASK = 0x3fbfcb1f;

// clang-format off
//
// Button indexes
//  0   1   2   3   4   5   6 |   7   8   9  10  11  12  13
// 14  15  16  17  18  19  20 |  21  22  23  24  25  26  27
// 28  29  30  31  32  33     |      34  35  36  37  38  39
//               40  41   42  |   43   44  45
//
//  Front:
// 18  17  12  11  04  03  21 |  44  26  27  34  35  40  41
// 19  16  13  10  05  02  22 |  45  25  28  33  36  39  42
// 20  15  14  09  06  01     |      24  29  32  37  38  43
//               08  07  00   |    23  30  31

constexpr uint8_t RGB_MAP[JAVELIN_RGB_COUNT] = {
  18, 17, 12, 11, 04, 03, 21, /**/ 44, 26, 27, 34, 35, 40, 41,
  19, 16, 13, 10, 05, 02, 22, /**/ 45, 25, 28, 33, 36, 39, 42,
  20, 15, 14,  9, 06, 01,     /**/     24, 29, 32, 37, 38, 43,
                  8,  7,  0,  /**/  23, 30, 31
};

// clang-format on

const size_t BUTTON_COUNT = 46;

#define JAVELIN_ENCODER 1
#define JAVELIN_ENCODER_COUNT 4
#define JAVELIN_ENCODER_LEFT_COUNT 2
#define JAVELIN_ENCODER_SPEED 2

constexpr EncoderPins ENCODER_PINS[] = {{5, 7}, {6, 7}, {24, 7}, {6, 7}};

const char *const MANUFACTURER_NAME = "foostan";
const char *const PRODUCT_NAME = "Corne (Javelin)";
const int VENDOR_ID = 0x4653;
// const int PRODUCT_ID = 0x0002;

//---------------------------------------------------------------------------
