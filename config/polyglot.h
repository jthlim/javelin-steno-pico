//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct StenoConfigBlock;
struct StenoOrthography;
struct StenoDictionaryCollection;

//---------------------------------------------------------------------------

#define JAVELIN_USE_EMBEDDED_STENO 1
#define JAVELIN_USE_USER_DICTIONARY 1
#define JAVELIN_USE_SCRIPT_STORAGE 1
#define JAVELIN_USB_MILLIAMPS 500

#define JAVELIN_RGB 1
#define JAVELIN_RGB_COUNT 32
#define JAVELIN_RGB_PIN 4

#define BOOTSEL_BUTTON_INDEX 42

#define JAVELIN_BUTTON_MATRIX 1

constexpr uint8_t COLUMN_PINS[] = {24, 23, 21, 20, 19, 18, 11, 10, 9, 8, 6, 5};
constexpr uint32_t COLUMN_PIN_MASK = 0x01bc0f60;
constexpr uint8_t ROW_PINS[] = {26, 25, 17, 16};
constexpr uint32_t ROW_PIN_MASK = 0x06030000;

// clang-format off
constexpr int8_t KEY_MAP[4][16] = {
  {  0,  1,  2,  3,  4,  5, /**/  6,  7,  8,  9, 10, 11 },
  { 12, 13, 14, 15, 16, 17, /**/ 18, 19, 20, 21, 22, 23 },
  { 24, 25, 26, 27, 28, 29, /**/ 30, 31, 32, 33, 34, 35 },
  { -1, -1, -1, 36, 37, 38, /**/ 39, 40, 41, -1, -1, -1 },
};
// clang-format on

#define JAVELIN_SCRIPT_CONFIGURATION                                           \
  R"({"name":"Polyglot","layout":[{"x":0,"y":0},{"x":1,"y":0},{"x":2,"y":0},{"x":3,"y":0},{"x":4,"y":0},{"x":5,"y":0},{"x":7,"y":0},{"x":8,"y":0},{"x":9,"y":0},{"x":10,"y":0},{"x":11,"y":0},{"x":12,"y":0},{"x":0,"y":1},{"x":1,"y":1},{"x":2,"y":1},{"x":3,"y":1},{"x":4,"y":1},{"x":5,"y":1},{"x":7,"y":1},{"x":8,"y":1},{"x":9,"y":1},{"x":10,"y":1},{"x":11,"y":1},{"x":12,"y":1},{"x":0,"y":2},{"x":1,"y":2},{"x":2,"y":2},{"x":3,"y":2},{"x":4,"y":2},{"x":5,"y":2},{"x":7,"y":2},{"x":8,"y":2},{"x":9,"y":2},{"x":10,"y":2},{"x":11,"y":2},{"x":12,"y":2},{"x":3,"y":3},{"x":4,"y":3},{"x":5,"y":3},{"x":7,"y":3},{"x":8,"y":3},{"x":9,"y":3},{"x":12.5,"y":3.5,"s":0.5}]})"

const StenoOrthography *const ORTHOGRAPHY_ADDRESS =
    (const StenoOrthography *)0x1003e000;
const uint8_t *const STENO_WORD_LIST_ADDRESS = (const uint8_t *)0x10040000;
static const struct ScriptStorageData *const SCRIPT_STORAGE_ADDRESS =
    (struct ScriptStorageData *)0x103de000;
static const size_t MAXIMUM_SCRIPT_STORAGE_SIZE = 0x20000;
const StenoConfigBlock *const STENO_CONFIG_BLOCK_ADDRESS =
    (const StenoConfigBlock *)0x103fe000;
const uint8_t *const SCRIPT_BYTE_CODE = (const uint8_t *)0x103fe100;
const StenoDictionaryCollection *const STENO_MAP_DICTIONARY_COLLECTION_ADDRESS =
    (const StenoDictionaryCollection *)0x10400000;
const uint8_t *const STENO_USER_DICTIONARY_ADDRESS =
    (const uint8_t *)0x10fc0000;
const size_t STENO_USER_DICTIONARY_SIZE = 0x40000;

const size_t MAXIMUM_MAP_DICTIONARY_SIZE = 0xbc0000;
const size_t MAXIMUM_BUTTON_SCRIPT_SIZE = 0x1f00;
const size_t BUTTON_COUNT = 43;

const char *const MANUFACTURER_NAME = "stenokeyboards";
const char *const PRODUCT_NAME = "The Polyglot (Javelin)";
const int VENDOR_ID = 0x9000;

//---------------------------------------------------------------------------
