//---------------------------------------------------------------------------

#pragma once
#include "javelin/button_state.h"
#include "javelin/container/cyclic_queue.h"
#include "javelin/debounce.h"

#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

class PicoButtonState {
public:
  static void Initialize();
  static void Update() { instance.UpdateInternal(); }

  static bool HasData() { return instance.queue.IsNotEmpty(); }
  static bool IsEmpty() { return instance.queue.IsEmpty(); }
  static bool IsNotEmpty() { return instance.queue.IsNotEmpty(); }
  static const TimedButtonState &Front() { return instance.queue.Front(); }
  static void RemoveFront() { instance.queue.RemoveFront(); }
  static size_t GetCount() { return instance.queue.GetCount(); }

  static void SetInInterrupt(bool isInInterrupt) {
#if defined(BOOTSEL_BUTTON_INDEX)
    instance.isInInterrupt = isInInterrupt;
#endif
  }

private:
  static constexpr size_t QUEUE_COUNT = 64;

#if defined(BOOTSEL_BUTTON_INDEX)
  bool lastBootSelButtonState;
  bool isInInterrupt;
#endif

  uint32_t keyPressedTime;
  GlobalDeferredDebounce<ButtonState> debouncer;
  CyclicQueue<TimedButtonState, QUEUE_COUNT> queue;

  static PicoButtonState instance;

  void UpdateInternal();

  static ButtonState ReadInternal();
  static void ReadTouchCounters(uint32_t *counters);
  static bool IsBootSelButtonPressed();
};

//---------------------------------------------------------------------------
