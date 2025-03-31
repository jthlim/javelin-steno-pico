//---------------------------------------------------------------------------

#pragma once
#include "pico_register.h"

//---------------------------------------------------------------------------

struct PicoTimer {
  PicoRegister timehw;
  PicoRegister timelw;
  const PicoRegister timehr;
  const PicoRegister timelr;
  PicoRegister alarms[4];
  PicoRegister armed;
  const PicoRegister timerawh;
  const PicoRegister timerawl;
  PicoRegister dbgPause;
  PicoRegister pause;
#if JAVELIN_PICO_PLATFORM == 2350
  PicoRegister locked;
  PicoRegister source;
#endif
  PicoRegister interrupt; // Raw Status
  PicoRegister interruptEnable;
  PicoRegister interruptForce;
  const PicoRegister interruptStatus; // After masking and forcing

  void EnableAlarmInterrupt(uint32_t alarmIndex) {
    interruptEnable |= 1 << alarmIndex;
  }
  void DisableAlarmInterrupt(uint32_t alarmIndex) {
    interruptEnable &= ~(1 << alarmIndex);
  }
  void AcknowledgeAlarmInterrupt(uint32_t alarmIndex) {
    interrupt &= ~(1 << alarmIndex);
  }
  void SetAlarmAfterDelayInMicroseconds(uint32_t alarmIndex, uint32_t delay) {
    alarms[alarmIndex] = timerawl + delay;
  }
};

#if JAVELIN_PICO_PLATFORM == 2040
static PicoTimer *const timer = (PicoTimer *)0x40054000;
#elif JAVELIN_PICO_PLATFORM == 2350
static PicoTimer *const timer = (PicoTimer *)0x400b0000;
static PicoTimer *const timer0 = (PicoTimer *)0x400b0000;
static PicoTimer *const timer1 = (PicoTimer *)0x400b8000;
#else
#error Unsupported platform
#endif

//---------------------------------------------------------------------------
