//---------------------------------------------------------------------------

#include JAVELIN_BOARD_CONFIG

#include "javelin/flash.h"
#include <hardware/flash.h>
#include <hardware/sync.h>
#include <string.h>

//---------------------------------------------------------------------------

extern "C" char __flash_binary_end[];

static bool IsWritableRange(const void *p) { return p >= __flash_binary_end; }

void Flash::EraseBlockInternal(const void *target, size_t size) {
  if (!IsWritableRange(target)) {
    return;
  }

  instance.erasedBytes += size;

  const uint32_t interrupts = save_and_disable_interrupts();
  flash_range_erase((intptr_t)target - XIP_BASE, size);
  restore_interrupts(interrupts);
}

void Flash::WriteBlockInternal(const void *target, const void *data,
                               size_t size) {
  if (!IsWritableRange(target)) {
    return;
  }

  for (int i = 0; i < 3; ++i) {
    instance.programmedBytes += size;

    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program((intptr_t)target - XIP_BASE, (const uint8_t *)data,
                        size);
    restore_interrupts(interrupts);

    if (memcmp(target, data, size) == 0) {
      return;
    }

    instance.reprogrammedBytes += size;
  }
}

//---------------------------------------------------------------------------

bool Flash::IsScriptMemory(const void *start, const void *end) {
  const void *byteCodeStart = SCRIPT_BYTE_CODE;
  const void *byteCodeEnd = SCRIPT_BYTE_CODE + MAXIMUM_BUTTON_SCRIPT_SIZE;

  // Intersect if Max(start, byteCodeStart) < Min(end, byteCodeEnd)
  return start < byteCodeEnd && byteCodeStart < end;
}

//---------------------------------------------------------------------------