//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct ScriptStorageData;
struct StenoConfigBlock;

//---------------------------------------------------------------------------

#define JAVELIN_USE_SCRIPT_STORAGE 1

//---------------------------------------------------------------------------

const StenoConfigBlock *const STENO_CONFIG_BLOCK_ADDRESS =
    (const StenoConfigBlock *)0x1003a000;
const uint8_t *const SCRIPT_BYTE_CODE = (const uint8_t *)0x1003a100;
const size_t MAXIMUM_BUTTON_SCRIPT_SIZE = 0x7f00;

static const struct ScriptStorageData *const SCRIPT_STORAGE_ADDRESS =
    (struct ScriptStorageData *)0x10050000;
static const size_t MAXIMUM_SCRIPT_STORAGE_SIZE = 0x20000;

static const intptr_t ASSET_STORAGE_START_ADDRESS = 0x10070000;
static const intptr_t ASSET_STORAGE_END_ADDRESS = 0x10100000;

//---------------------------------------------------------------------------
