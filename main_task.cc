//---------------------------------------------------------------------------

#include "main_task.h"
#include "javelin/button_script_manager.h"
#include "javelin/button_state.h"
#include "javelin/clock.h"
#include "javelin/console.h"
#include "javelin/flash.h"
#include "javelin/split/split_key_state.h"
#include "javelin/timer_manager.h"
#include "pinnacle.h"
#include "rp2040_button_state.h"
#include "rp2040_encoder_state.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
JavelinStaticAllocate<SlaveTask> SlaveTask::container;
#else
JavelinStaticAllocate<MasterTask> MasterTask::container;
#endif

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
void MasterTask::UpdateBuffer(TxBuffer &buffer) {
  if (timestampState == TimestampState::REQUEST_TIMESTAMP) {
    timestampState = TimestampState::WAITING_FOR_RESPONSE;

    const uint32_t now = Clock::GetMilliseconds();
    buffer.Add(SplitHandlerId::KEY_STATE, &now, sizeof(uint32_t));
  }
}

void MasterTask::OnTransmitConnected() {
  timestampState = TimestampState::REQUEST_TIMESTAMP;
}

void MasterTask::OnReceiveConnectionReset() {
  shouldProcessQueue = true;
  timestampState = TimestampState::DISCONNECTED;
  slaveState.ClearAll();
  slaveStates.Reset();
}

void MasterTask::OnDataReceived(const void *data, size_t length) {
  shouldProcessQueue = true;

  const SplitKeyState *transmitData = (SplitKeyState *)data;
  slaveTimestamp = transmitData->GetTimestamp();

  if (transmitData->HasButtonState()) {
    TimedButtonState &timedButtonState = slaveStates.Add();
    timedButtonState.timestamp = slaveTimestamp;
    timedButtonState.state = transmitData->GetButtonState(length);
  }

  if (timestampState == TimestampState::WAITING_FOR_RESPONSE) {
    timestampState = TimestampState::RESPONSE_RECEIVED;
  }
}
#endif

#if JAVELIN_SPLIT
inline bool MasterTask::IsWaitingForSlaveTimestamp() const {
  switch (timestampState) {
  case TimestampState::DISCONNECTED:
  case TimestampState::INACTIVE:
    return false;

  case TimestampState::REQUEST_TIMESTAMP:
  case TimestampState::WAITING_FOR_RESPONSE:
  case TimestampState::RESPONSE_RECEIVED:
    return true;
  }
  return true;
}

inline void MasterTask::RequestSlaveTimestamp() {
  switch (timestampState) {
  case TimestampState::DISCONNECTED:
    shouldProcessQueue = true;
    break;

  case TimestampState::INACTIVE:
    timestampState = TimestampState::REQUEST_TIMESTAMP;
    break;

  case TimestampState::REQUEST_TIMESTAMP:
  case TimestampState::WAITING_FOR_RESPONSE:
  case TimestampState::RESPONSE_RECEIVED:
    break;
  }
}
#endif

inline uint32_t MasterTask::GetScriptTime() const {
#if JAVELIN_SPLIT
  if (slaveStates.IsNotEmpty()) {
    if (Rp2040ButtonState::IsEmpty() ||
        slaveStates.Front().timestamp <= Rp2040ButtonState::Front().timestamp) {
      return slaveStates.Front().timestamp;
    } else {
      return Rp2040ButtonState::Front().timestamp;
    }
  } else if (Rp2040ButtonState::IsNotEmpty()) {
    return Rp2040ButtonState::Front().timestamp;
  }
#else
  if (Rp2040ButtonState::IsNotEmpty()) {
    return Rp2040ButtonState::Front().timestamp;
  }
#endif
  return Clock::GetMilliseconds();
}

void MasterTask::UpdateQueue() {
  if (!Rp2040ButtonState::HasData()) {
    return;
  }

#if JAVELIN_SPLIT
  RequestSlaveTimestamp();

  // TODO: Jan 2026, remove this. This is to show the pair firmware
  // needs updating message after switching to keypress queues.
  if (slaveTimestamp == 0) {
    shouldProcessQueue = true;
  }

#else
  shouldProcessQueue = true;
#endif
}

void MasterTask::Update() {
  if (Flash::IsUpdating()) {
    return;
  }

  UpdateQueue();
  Pinnacle::Update();
  Rp2040EncoderState::Update();

  const uint32_t scriptTime = GetScriptTime();

  if (shouldProcessQueue) {
    shouldProcessQueue = false;
    ProcessQueue();
  }

  ButtonScriptManager::GetInstance().Tick(scriptTime);
  TimerManager::instance.ProcessTimers(scriptTime);
}

void MasterTask::ProcessQueue() {
  for (;;) {
#if JAVELIN_SPLIT
    uint32_t scriptTime;

    if (slaveStates.IsNotEmpty()) {
      if (Rp2040ButtonState::IsEmpty() ||
          Rp2040ButtonState::Front().timestamp >=
              slaveStates.Front().timestamp) {
        const TimedButtonState &front = slaveStates.RemoveFront();
        scriptTime = front.timestamp;
        slaveState = front.state;
      } else {
        const TimedButtonState &front = Rp2040ButtonState::Front();
        scriptTime = front.timestamp;
        localState = front.state;
        Rp2040ButtonState::RemoveFront();
      }
    } else if (Rp2040ButtonState::IsNotEmpty()) {
      if (IsWaitingForSlaveTimestamp() &&
          Rp2040ButtonState::Front().timestamp > slaveTimestamp &&
          (Rp2040ButtonState::Front().timestamp - slaveTimestamp < 2048)) {
        break;
      }
      const TimedButtonState &front = Rp2040ButtonState::Front();
      scriptTime = front.timestamp;
      localState = front.state;
      Rp2040ButtonState::RemoveFront();
    } else {
      break;
    }

    const ButtonState currentState = localState | slaveState;
    ButtonScriptManager::GetInstance().Update(currentState, scriptTime);
#else
    if (Rp2040ButtonState::IsEmpty()) {
      return;
    }

    const TimedButtonState &front = Rp2040ButtonState::Front();
    ButtonScriptManager::GetInstance().Update(front.state, front.timestamp);
    Rp2040ButtonState::RemoveFront();
#endif
  }

#if JAVELIN_SPLIT
  switch (timestampState) {
  case TimestampState::RESPONSE_RECEIVED:
    if (Rp2040ButtonState::IsNotEmpty()) {
      timestampState = TimestampState::REQUEST_TIMESTAMP;
    } else {
      timestampState = TimestampState::INACTIVE;
    }
    break;

  case TimestampState::INACTIVE:
    if (Rp2040ButtonState::IsNotEmpty()) {
      timestampState = TimestampState::REQUEST_TIMESTAMP;
    }
    break;

  case TimestampState::DISCONNECTED:
  case TimestampState::REQUEST_TIMESTAMP:
  case TimestampState::WAITING_FOR_RESPONSE:
    break;
  }
#endif
}

void MasterTask::PrintInfo() const {
  Console::Printf("Master Task\n");
  Console::Printf("  Script time: %u ms\n", GetScriptTime());
  Console::Printf("  Local state queue: %zu entries\n",
                  Rp2040ButtonState::GetCount());
#if JAVELIN_SPLIT
  Console::Printf("  Split state queue: %zu entries\n", slaveStates.GetCount());
#endif
}

//---------------------------------------------------------------------------

void SlaveTask::OnTransmitConnected() {
  updateMasterTimestampOffset = true;
  timestampOffsetIndex = 0;
  buffers.Reset();
}

void SlaveTask::OnDataReceived(const void *data, size_t length) {
  needsTransmit = true;
  const uint32_t masterTimestamp = *(uint32_t *)data;
  const int32_t offset = masterTimestamp - Clock::GetMilliseconds();

  timestampOffsets[timestampOffsetIndex++ & (TIMESTAMP_HISTORY_COUNT - 1)] =
      offset;

  if (timestampOffsetIndex % 64 == 0) {
    // To avoid clock drift between the halves causing sync issues,
    // re-evaluate the masterTimestampOffset every 64 samples.
    masterTimestampOffset = timestampOffsets[0];
    for (size_t i = 1; i < TIMESTAMP_HISTORY_COUNT; ++i) {
      if (timestampOffsets[i] > masterTimestampOffset) {
        masterTimestampOffset = timestampOffsets[i];
      }
    }
  } else if (updateMasterTimestampOffset) {
    masterTimestampOffset = offset;
    updateMasterTimestampOffset = false;
  } else if (offset > masterTimestampOffset) {
    masterTimestampOffset = offset;
  }
}

void SlaveTask::Update() {
  if (Flash::IsUpdating()) {
    return;
  }

  Pinnacle::Update();
  Rp2040EncoderState::Update();

  const uint32_t scriptTime = Clock::GetMilliseconds();
  ButtonScriptManager::GetInstance().Tick(scriptTime);
  TimerManager::instance.ProcessTimers(scriptTime);
  UpdateQueue();
}

void SlaveTask::UpdateQueue() {
  if (!Rp2040ButtonState::HasData()) {
    return;
  }

  do {
    const TimedButtonState &buttonStateEntry = Rp2040ButtonState::Front();

    if (!buffers.IsFull()) {
      buffers.Add(buttonStateEntry);
    }

    ButtonScriptManager::GetInstance().Update(buttonStateEntry.state,
                                              buttonStateEntry.timestamp);
    Rp2040ButtonState::RemoveFront();
  } while (Rp2040ButtonState::HasData());
}

void SlaveTask::UpdateBuffer(TxBuffer &buffer) {
  if (buffers.IsNotEmpty()) {
    needsTransmit = false;

    do {
      const TimedButtonState &entry = buffers.RemoveFront();

      SplitKeyState transmitData;
      const size_t dataSize = transmitData.Set(
          entry.timestamp + masterTimestampOffset, entry.state);
      buffer.Add(SplitHandlerId::KEY_STATE, &transmitData, dataSize);
    } while ((buffers.IsNotEmpty()));
  } else if (needsTransmit) {
    needsTransmit = false;
    SplitKeyState transmitData;
    const size_t dataSize =
        transmitData.Set(Clock::GetMilliseconds() + masterTimestampOffset);
    buffer.Add(SplitHandlerId::KEY_STATE, &transmitData, dataSize);
  }
}

//---------------------------------------------------------------------------
