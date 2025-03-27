//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

template <size_t N> constexpr uint32_t CALCULATE_MASK(const uint8_t (&v)[N]) {
  uint32_t result = 0;
  for (const uint8_t x : v) {
    const uint8_t pin = x & 0x7f;
    if (pin != 0x7f) {
      result |= (1 << pin);
    }
  }
  return result;
}

//---------------------------------------------------------------------------
