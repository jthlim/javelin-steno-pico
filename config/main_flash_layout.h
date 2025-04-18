//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

struct HostLayouts;
struct ScriptStorageData;
struct StenoConfigBlock;
struct StenoSystem;
struct StenoDictionaryCollection;

//---------------------------------------------------------------------------

#define JAVELIN_USE_SCRIPT_STORAGE 1

//---------------------------------------------------------------------------

const StenoConfigBlock *const STENO_CONFIG_BLOCK_ADDRESS =
    (const StenoConfigBlock *)0x1004a000;
const uint8_t *const SCRIPT_BYTE_CODE = (const uint8_t *)0x1004a100;
const StenoSystem *const SYSTEM_ADDRESS = (const StenoSystem *)0x10052000;
const uint8_t *const STENO_WORD_LIST_ADDRESS = (const uint8_t *)0x10054000;
static const struct HostLayouts *const HOST_LAYOUTS_ADDRESS =
    (struct HostLayouts *)0x103de000;
static const struct ScriptStorageData *const SCRIPT_STORAGE_ADDRESS =
    (struct ScriptStorageData *)0x103e0000;
static const size_t MAXIMUM_SCRIPT_STORAGE_SIZE = 0x20000;
const StenoDictionaryCollection *const STENO_MAP_DICTIONARY_COLLECTION_ADDRESS =
    (const StenoDictionaryCollection *)0x10400000;
const intptr_t ASSET_STORAGE_END_ADDRESS = 0x10fc0000;
const uint8_t *const STENO_USER_DICTIONARY_ADDRESS =
    (const uint8_t *)0x10fc0000;
const size_t STENO_USER_DICTIONARY_SIZE = 0x40000;

const size_t MAXIMUM_MAP_DICTIONARY_SIZE = 0xbc0000;
const size_t MAXIMUM_BUTTON_SCRIPT_SIZE = 0x7f00;

//---------------------------------------------------------------------------
