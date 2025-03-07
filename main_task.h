//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include "javelin/button_state.h"
#include "javelin/container/cyclic_queue.h"
#include "javelin/debounce.h"
#include "javelin/split/split.h"
#include "javelin/static_allocate.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
class MasterTask final : public SplitTxHandler, public SplitRxHandler {
#else
class MasterTask {
#endif
public:
  void Update();
  void UpdateQueue();

  void PrintInfo() const;

  static JavelinStaticAllocate<MasterTask> container;

private:
  static constexpr size_t BUFFER_COUNT = 64;

  bool shouldProcessQueue;

#if JAVELIN_SPLIT
  enum class TimestampState : uint8_t {
    DISCONNECTED,
    INACTIVE,
    REQUEST_TIMESTAMP,
    WAITING_FOR_RESPONSE,
    RESPONSE_RECEIVED,
  };

  TimestampState timestampState;
  uint32_t slaveTimestamp;
  ButtonState slaveState;

  CyclicQueue<TimedButtonState, BUFFER_COUNT> slaveStates;
#endif
  ButtonState localState;

  // TODO: Remove.
  GlobalDeferredDebounce<ButtonState> debouncer;

  void ProcessQueue();
  uint32_t GetScriptTime() const;

#if JAVELIN_SPLIT
  bool IsWaitingForSlaveTimestamp() const;
  void RequestSlaveTimestamp();

  void UpdateBuffer(TxBuffer &buffer) final;
  void OnTransmitConnected() final;
  void OnReceiveConnectionReset() final;
  void OnDataReceived(const void *data, size_t length) final;
#endif
};

class SlaveTask final : public SplitTxHandler, public SplitRxHandler {
public:
  void Update();
  void UpdateBuffer(TxBuffer &buffer) final;

  static JavelinStaticAllocate<SlaveTask> container;

private:
  static constexpr size_t TIMESTAMP_HISTORY_COUNT = 64;

  bool needsTransmit;
  bool updateMasterTimestampOffset;
  uint8_t timestampOffsetIndex;
  int32_t masterTimestampOffset;
  int32_t timestampOffsets[TIMESTAMP_HISTORY_COUNT];

  static constexpr size_t BUFFER_COUNT = 32;
  CyclicQueue<TimedButtonState, BUFFER_COUNT> buffers;

  // Temp -- remove.
  GlobalDeferredDebounce<ButtonState> debouncer;

  void UpdateQueue();

  void OnTransmitConnected() final;
  void OnDataReceived(const void *data, size_t length) final;
};

//---------------------------------------------------------------------------
