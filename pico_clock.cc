//---------------------------------------------------------------------------

#include "javelin/clock.h"
#include <hardware/timer.h>
#include <tusb.h>

//---------------------------------------------------------------------------

uint32_t Clock::GetMilliseconds() { return (time_us_64() * 4294968) >> 32; }

uint32_t Clock::GetMicroseconds() { return time_us_32(); }

void Clock::Sleep(uint32_t milliseconds) {
  const uint32_t startTime = Clock::GetMicroseconds();
  const uint32_t microseconds = 1000 * milliseconds;
  while (Clock::GetMicroseconds() - startTime < microseconds) {
    sleep_us(100);
    tud_task();
#if JAVELIN_USE_WATCHDOG
    watchdog_update();
#endif
  }
}
//---------------------------------------------------------------------------
