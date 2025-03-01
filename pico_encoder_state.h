//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include "javelin/debounce.h"
#include "javelin/split/split.h"

//---------------------------------------------------------------------------

#if JAVELIN_ENCODER

class PicoEncoderState
#if JAVELIN_SPLIT
#if JAVELIN_SPLIT_IS_MASTER
    : public SplitRxHandler
#else
    : public SplitTxHandler
#endif
#endif
{
public:
  static void Initialize() { instance.InitializeInternal(); }

  static size_t GetLocalEncoderCount() { return instance.localEncoderCount; }
  static void SetLocalEncoderCount(size_t value) {
    instance.localEncoderCount = value;
  }

  static void Update() { instance.UpdateInternal(); }
  static void UpdateNoScriptCall() { instance.UpdateNoScriptCallInternal(); }

  static void RegisterTxHandler() {
#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
    Split::RegisterTxHandler(&instance);
#endif
  }
  static void RegisterRxHandler() {
#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
    Split::RegisterRxHandler(SplitHandlerId::ENCODER, &instance);
#endif
  }

  bool SetConfiguration(size_t encoderIndex, const int8_t *data);
  static void SetEncoderConfiguration_Binding(void *context,
                                              const char *commandLine);

private:
  PicoEncoderState();

  uint8_t localEncoderCount;
#if JAVELIN_SPLIT
  uint8_t localEncoderOffset;
#else
  static const size_t localEncoderOffset = 0;
#endif

  int8_t deltas[JAVELIN_ENCODER_COUNT];
  int8_t lastLUT[JAVELIN_ENCODER_COUNT];
#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
  int8_t lastDeltas[JAVELIN_ENCODER_COUNT];
#endif
  GlobalDeferredDebounce<uint8_t, 1> lastEncoderStates[JAVELIN_ENCODER_COUNT];
  int8_t encoderLUT[JAVELIN_ENCODER_COUNT][16];

  static PicoEncoderState instance;

  void InitializeInternal();
  void UpdateInternal();
  void UpdateNoScriptCallInternal();

  bool HasAnyDelta() const;

  void OnDataReceived(const void *data, size_t length);
  void UpdateBuffer(TxBuffer &buffer);
  void OnReceiveConnected();

  void CallScript(size_t encoderIndex, int delta);

  void SendConfigurationInfoToPair(size_t encoderIndex);
};

#else

class PicoEncoderState {
public:
  static void Initialize() {}
  static void Update() {}
  static void UpdateNoScriptCall() {}
  static void RegisterTxHandler() {}
  static void RegisterRxHandler() {}
};

#endif

//---------------------------------------------------------------------------
