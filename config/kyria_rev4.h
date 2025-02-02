//---------------------------------------------------------------------------

#pragma once
#include "encoder_pins.h"
#include "main_flash_layout.h"

//---------------------------------------------------------------------------

// The cirque driver has auto-detection, so can always be enabled.
#define USE_CIRQUE_TRACKPAD 1

// There is no way to autodetect display/encoder, so only one of these can
// be enabled.
#define USE_HALCYON_DISPLAY 1
#define USE_HALCYON_ENCODER 0

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USB_MILLIAMPS 500

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 62
#define JAVELIN_RGB_LEFT_COUNT 31
#define JAVELIN_RGB_PIN 3
#define JAVELIN_USE_RGB_MAP 1

#define JAVELIN_SPLIT 1
#define JAVELIN_SPLIT_TX_PIN 28
#define JAVELIN_SPLIT_RX_PIN 29
#define JAVELIN_SPLIT_IS_MASTER 1
#define JAVELIN_SPLIT_SIDE_PIN 24
#define JAVELIN_SPLIT_TX_RX_BUFFER_SIZE 2048

#if USE_HALCYON_DISPLAY
#define JAVELIN_DISPLAY_DRIVER 7789
#define JAVELIN_DISPLAY_MISO_PIN 12
#define JAVELIN_DISPLAY_CS_PIN 13
#define JAVELIN_DISPLAY_SCK_PIN 14
#define JAVELIN_DISPLAY_MOSI_PIN 15
#define JAVELIN_DISPLAY_DC_PIN 16
#define JAVELIN_DISPLAY_RST_PIN 26
#define JAVELIN_DISPLAY_SPI spi1
#define JAVELIN_DISPLAY_SCREEN_WIDTH 135
#define JAVELIN_DISPLAY_SCREEN_HEIGHT 240
#define JAVELIN_DISPLAY_ROTATION 0
#define JAVELIN_DISPLAY_WIDTH 135
#define JAVELIN_DISPLAY_HEIGHT 240
#endif

#if USE_CIRQUE_TRACKPAD
#define JAVELIN_POINTER 0x73a
#define JAVELIN_POINTER_SPI spi1
#define JAVELIN_POINTER_MISO_PIN 12
#define JAVELIN_POINTER_CS_PIN 13
#define JAVELIN_POINTER_SCK_PIN 14
#define JAVELIN_POINTER_MOSI_PIN 15

#define JAVELIN_POINTER_INVERT_X 1
#define JAVELIN_POINTER_INVERT_Y 1

#define JAVELIN_POINTER_COUNT 2
#define JAVELIN_POINTER_LOCAL_OFFSET 0
#endif

#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t LEFT_COLUMN_PINS[] = {19, 20, 25, 4, 9, 10, 5};
constexpr uint32_t LEFT_COLUMN_PIN_MASK = 0x02180630;
constexpr uint8_t LEFT_ROW_PINS[] = {8, 11, 7, 6};
constexpr uint32_t LEFT_ROW_PIN_MASK = 0x000009c0;

constexpr uint8_t RIGHT_COLUMN_PINS[] = {5, 10, 9, 4, 25, 20, 19};
constexpr uint32_t RIGHT_COLUMN_PIN_MASK = 0x02180630;
constexpr uint8_t RIGHT_ROW_PINS[] = {8, 11, 7, 6};
constexpr uint32_t RIGHT_ROW_PIN_MASK = 0x000009c0;

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

#if USE_HALCYON_ENCODER
#define JAVELIN_BUTTON_PINS 1
#define JAVELIN_BUTTON_PINS_OFFSET 50
constexpr uint32_t BUTTON_PIN_MASK = 0x00010000;
constexpr uint8_t BUTTON_PINS[] = {16};

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Kyria rev4","layout":[{"x":0,"y":0.9},{"x":1,"y":0.9},{"x":2,"y":0.35},{"x":3,"y":0},{"x":4,"y":0.35},{"x":5,"y":0.5},{"x":12,"y":0.5},{"x":13,"y":0.35},{"x":14,"y":0},{"x":15,"y":0.35},{"x":16,"y":0.9},{"x":17,"y":0.9},{"x":0,"y":1.9},{"x":1,"y":1.9},{"x":2,"y":1.35},{"x":3,"y":1},{"x":4,"y":1.35},{"x":5,"y":1.5},{"x":12,"y":1.5},{"x":13,"y":1.35},{"x":14,"y":1},{"x":15,"y":1.35},{"x":16,"y":1.9},{"x":17,"y":1.9},{"x":0,"y":2.9},{"x":1,"y":2.9},{"x":2,"y":2.35},{"x":3,"y":2},{"x":4,"y":2.35},{"x":5,"y":2.5},{"x":6.12,"y":3.13,"r":0.52},{"x":7.21,"y":3.99,"r":0.79},{"x":9.79,"y":3.99,"r":-0.79},{"x":10.88,"y":3.13,"r":-0.52},{"x":12,"y":2.5},{"x":13,"y":2.35},{"x":14,"y":2},{"x":15,"y":2.35},{"x":16,"y":2.9},{"x":17,"y":2.9},{"x":2.5,"y":3.4},{"x":3.5,"y":3.4},{"x":4.6,"y":3.55,"r":0.26},{"x":5.62,"y":4,"r":0.52},{"x":6.5,"y":4.7,"r":0.79},{"x":10.5,"y":4.7,"r":-0.79},{"x":11.38,"y":4,"r":-0.52},{"x":12.4,"y":3.55,"r":-0.26},{"x":13.5,"y":3.4},{"x":14.5,"y":3.4},{"x":6.2,"y":1.4,"s":0.8},{"x":11,"y":1.4,"s":0.8}]})"

const size_t BUTTON_COUNT = 52;

constexpr EncoderPins ENCODER_PINS[] = {{23, 22}, {27, 26}};

#else

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Kyria rev4","layout":[{"x":0,"y":0.9},{"x":1,"y":0.9},{"x":2,"y":0.35},{"x":3,"y":0},{"x":4,"y":0.35},{"x":5,"y":0.5},{"x":12,"y":0.5},{"x":13,"y":0.35},{"x":14,"y":0},{"x":15,"y":0.35},{"x":16,"y":0.9},{"x":17,"y":0.9},{"x":0,"y":1.9},{"x":1,"y":1.9},{"x":2,"y":1.35},{"x":3,"y":1},{"x":4,"y":1.35},{"x":5,"y":1.5},{"x":12,"y":1.5},{"x":13,"y":1.35},{"x":14,"y":1},{"x":15,"y":1.35},{"x":16,"y":1.9},{"x":17,"y":1.9},{"x":0,"y":2.9},{"x":1,"y":2.9},{"x":2,"y":2.35},{"x":3,"y":2},{"x":4,"y":2.35},{"x":5,"y":2.5},{"x":6.12,"y":3.13,"r":0.52},{"x":7.21,"y":3.99,"r":0.79},{"x":9.79,"y":3.99,"r":-0.79},{"x":10.88,"y":3.13,"r":-0.52},{"x":12,"y":2.5},{"x":13,"y":2.35},{"x":14,"y":2},{"x":15,"y":2.35},{"x":16,"y":2.9},{"x":17,"y":2.9},{"x":2.5,"y":3.4},{"x":3.5,"y":3.4},{"x":4.6,"y":3.55,"r":0.26},{"x":5.62,"y":4,"r":0.52},{"x":6.5,"y":4.7,"r":0.79},{"x":10.5,"y":4.7,"r":-0.79},{"x":11.38,"y":4,"r":-0.52},{"x":12.4,"y":3.55,"r":-0.26},{"x":13.5,"y":3.4},{"x":14.5,"y":3.4}]})"

const size_t BUTTON_COUNT = 50;

constexpr EncoderPins ENCODER_PINS[] = {{23, 22}};

#endif

#define JAVELIN_ENCODER 1
#define JAVELIN_ENCODER_COUNT 4
#define JAVELIN_ENCODER_LOCAL_OFFSET 0

const char *const MANUFACTURER_NAME = "splitkb";
const char *const PRODUCT_NAME = "Halcyon Kyria (Javelin)";
const int VENDOR_ID = 0x7fce;
// const int PRODUCT_ID = 0xcf44;

//---------------------------------------------------------------------------
