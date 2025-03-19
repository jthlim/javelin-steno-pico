//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct PicoSpinlock {
public:
  void Lock() {
    while (spinlock == 0) {
      // Busy wait.
    }
  }

  void Unlock() { spinlock = (intptr_t)(this); }

private:
  volatile uint32_t spinlock;
};

//---------------------------------------------------------------------------

static PicoSpinlock *const spinlock0 = (PicoSpinlock *)0xd0000100;
static PicoSpinlock *const spinlock1 = (PicoSpinlock *)0xd0000104;
static PicoSpinlock *const spinlock2 = (PicoSpinlock *)0xd0000108;
static PicoSpinlock *const spinlock3 = (PicoSpinlock *)0xd000010c;
static PicoSpinlock *const spinlock4 = (PicoSpinlock *)0xd0000110;
static PicoSpinlock *const spinlock5 = (PicoSpinlock *)0xd0000114;
static PicoSpinlock *const spinlock6 = (PicoSpinlock *)0xd0000118;
static PicoSpinlock *const spinlock7 = (PicoSpinlock *)0xd000011c;
static PicoSpinlock *const spinlock8 = (PicoSpinlock *)0xd0000120;
static PicoSpinlock *const spinlock9 = (PicoSpinlock *)0xd0000124;
static PicoSpinlock *const spinlock10 = (PicoSpinlock *)0xd0000128;
static PicoSpinlock *const spinlock11 = (PicoSpinlock *)0xd000012c;
static PicoSpinlock *const spinlock12 = (PicoSpinlock *)0xd0000130;
static PicoSpinlock *const spinlock13 = (PicoSpinlock *)0xd0000134;
static PicoSpinlock *const spinlock14 = (PicoSpinlock *)0xd0000138;
static PicoSpinlock *const spinlock15 = (PicoSpinlock *)0xd000013c;
static PicoSpinlock *const spinlock16 = (PicoSpinlock *)0xd0000140;
static PicoSpinlock *const spinlock17 = (PicoSpinlock *)0xd0000144;
static PicoSpinlock *const spinlock18 = (PicoSpinlock *)0xd0000148;
static PicoSpinlock *const spinlock19 = (PicoSpinlock *)0xd000014c;
static PicoSpinlock *const spinlock20 = (PicoSpinlock *)0xd0000150;
static PicoSpinlock *const spinlock21 = (PicoSpinlock *)0xd0000154;
static PicoSpinlock *const spinlock22 = (PicoSpinlock *)0xd0000158;
static PicoSpinlock *const spinlock23 = (PicoSpinlock *)0xd000015c;
static PicoSpinlock *const spinlock24 = (PicoSpinlock *)0xd0000160;
static PicoSpinlock *const spinlock25 = (PicoSpinlock *)0xd0000164;
static PicoSpinlock *const spinlock26 = (PicoSpinlock *)0xd0000168;
static PicoSpinlock *const spinlock27 = (PicoSpinlock *)0xd000016c;
static PicoSpinlock *const spinlock28 = (PicoSpinlock *)0xd0000170;
static PicoSpinlock *const spinlock29 = (PicoSpinlock *)0xd0000174;
static PicoSpinlock *const spinlock30 = (PicoSpinlock *)0xd0000178;
static PicoSpinlock *const spinlock31 = (PicoSpinlock *)0xd000017c;

//---------------------------------------------------------------------------
