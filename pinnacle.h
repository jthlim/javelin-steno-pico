//---------------------------------------------------------------------------
// Driver for Cirque.
//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include "javelin/split/split.h"
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct Pointer {
  int x;
  int y;
  int z;

  bool operator==(const Pointer &other) const = default;
};

//---------------------------------------------------------------------------

// Pinnacle devices return ID 0x73a, so use that as the ID
#if JAVELIN_POINTER == 0x73a

enum class PinnacleRegister : int;

// Driver for Cirque touchpads.
class Pinnacle
#if JAVELIN_SPLIT
#if JAVELIN_SPLIT_IS_MASTER
    : public SplitRxHandler
#else
    : public SplitTxHandler
#endif
#endif
{
public:
  static void Initialize();
  static void Uninitialize();

  static void Update() { instance.UpdateInternal(); }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }
  static void RegisterRxHandler() {
#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::POINTER, &instance);
#endif
  }

  static void WriteRegister(PinnacleRegister reg, int value);
  static int ReadRegister(PinnacleRegister reg);
  static void ReadRegisters(PinnacleRegister startingReg, uint8_t *result,
                            size_t length);

  static void ClearFlags();

private:
  bool available;
  struct Data {
    bool isDirty;
    Pointer pointer;
  };
  Data data[JAVELIN_POINTER_COUNT];

  void UpdateInternal();

  void OnDataReceived(const void *data, size_t length);
  void UpdateBuffer(TxBuffer &buffer);

  static void CallScript(size_t pointerIndex, const Pointer &pointer);

  static Pinnacle instance;
};

#else

struct Pinnacle {
public:
  static void Initialize() {}
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}

  static void Update() {}
};

#endif

//---------------------------------------------------------------------------
