//---------------------------------------------------------------------------

#pragma once
#include "javelin/split/split.h"
#include <hardware/pio.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

class PicoSplit : public Split {
public:
  static void Initialize() { instance.Initialize(); }
  static void Update() { instance.Update(); }
  static void PrintInfo() { instance.PrintInfo(); }
  static bool IsPairConnected() { return instance.isConnected; }

private:
  struct SplitData {
    enum class State : uint8_t {
      READY_TO_SEND,
      SENDING,
      RECEIVING,
    };

    SplitData();

    State state;
    bool updateSendData = true;
    bool isConnected = false;
    uint8_t retryCount;
    uint8_t programOffset;
    uint16_t txId;
    uint16_t lastRxId;
    uint32_t receiveStartTime;
    uint64_t rxPacketCount;
    uint64_t txIrqCount;
    uint64_t rxWords;
    uint64_t txWords;

#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
    pio_sm_config config;
#else
    pio_sm_config txConfig;
    pio_sm_config rxConfig;
#endif

    size_t metrics[SplitMetricId::COUNT];
    TxBuffer txBuffer;
    RxBuffer rxBuffer;

    void Initialize();
    void StartTx();
    void StartRx();

    void SendData();
    void SendTxBuffer();
    void Update();
    void ResetRxDma();
    void OnReceiveFailed();
    void OnReceiveTimeout();
    void OnReceiveSucceeded();

    bool ProcessReceive();

    static void TxIrqHandler();

    void PrintInfo();
  };

  static SplitData instance;
};

#else

class PicoSplit : public Split {
public:
  static void Initialize() {}
  static void Update() {}
  static void PrintInfo() {}
  static bool IsPairConnected() { return false; }
};

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------