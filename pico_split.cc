//---------------------------------------------------------------------------

#include "pico_split.h"

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

#include "javelin/button_script_manager.h"
#include "javelin/console.h"
#include "pico_dma.h"
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>
#include <hardware/timer.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT_PIO_CYCLES == 20
#include "pico_split_20.pio.h"
#elif JAVELIN_SPLIT_PIO_CYCLES == 16
#include "pico_split_16.pio.h"
#elif JAVELIN_SPLIT_PIO_CYCLES == 10
#include "pico_split_10.pio.h"
#elif JAVELIN_SPLIT_PIO_CYCLES == 8
#include "pico_split_8.pio.h"
#elif JAVELIN_SPLIT_PIO_CYCLES == 5
#include "pico_split_5.pio.h"
#else
#error Unsupported JAVELIN_SPLIT_PIO_CYCLES
#endif

//---------------------------------------------------------------------------

PicoSplit::SplitData PicoSplit::instance;

//---------------------------------------------------------------------------

#if !defined(JAVELIN_SPLIT_IS_LEFT)
bool Split::IsLeft() {
#if defined(JAVELIN_SPLIT_SIDE_PIN)
  return gpio_get(JAVELIN_SPLIT_SIDE_PIN);
#endif
  return false;
}
#endif

//---------------------------------------------------------------------------

const PIO PIO_INSTANCE = pio0;

#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
constexpr int TX_STATE_MACHINE_INDEX = 0;
constexpr int RX_STATE_MACHINE_INDEX = 0;
#else
constexpr int TX_STATE_MACHINE_INDEX = 0;
constexpr int RX_STATE_MACHINE_INDEX = 1;
#endif

constexpr uint32_t MASTER_RECEIVE_TIMEOUT_US = 2000;
constexpr uint32_t SLAVE_RECEIVE_TIMEOUT_US = 10000;
constexpr uint32_t RETRY_TIMEOUT_US = 100000;

//---------------------------------------------------------------------------

PicoSplit::SplitData::SplitData() {
  state = IsMaster() ? State::READY_TO_SEND : State::RECEIVING;
}

void PicoSplit::SplitData::Initialize() {
#if !defined(JAVELIN_SPLIT_IS_LEFT) && defined(JAVELIN_SPLIT_SIDE_PIN)
  gpio_init(JAVELIN_SPLIT_SIDE_PIN);
  gpio_set_dir(JAVELIN_SPLIT_SIDE_PIN, false);
#endif

  programOffset = pio_add_program(PIO_INSTANCE, &pico_split_program);

#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
  gpio_init(JAVELIN_SPLIT_RX_PIN);
  gpio_pull_down(JAVELIN_SPLIT_RX_PIN);
  gpio_set_function(JAVELIN_SPLIT_RX_PIN, GPIO_FUNC_PIO0);

  config = pico_split_program_get_default_config(programOffset);
  sm_config_set_in_pins(&config, JAVELIN_SPLIT_RX_PIN);
  sm_config_set_jmp_pin(&config, JAVELIN_SPLIT_RX_PIN);
  sm_config_set_out_pins(&config, JAVELIN_SPLIT_TX_PIN, 1);
  sm_config_set_set_pins(&config, JAVELIN_SPLIT_TX_PIN, 1);
  sm_config_set_sideset_pins(&config, JAVELIN_SPLIT_TX_PIN);
  sm_config_set_in_shift(&config, true, true, 32);
  sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_NONE);
  sm_config_set_out_shift(&config, true, true, 32);
  sm_config_set_clkdiv_int_frac(&config, 1, 0);
#else
  gpio_init(JAVELIN_SPLIT_TX_PIN);
  gpio_set_dir(JAVELIN_SPLIT_TX_PIN, GPIO_OUT);
  gpio_set_function(JAVELIN_SPLIT_TX_PIN, GPIO_FUNC_PIO0);

  gpio_init(JAVELIN_SPLIT_RX_PIN);
  gpio_set_dir(JAVELIN_SPLIT_RX_PIN, GPIO_IN);
  gpio_pull_down(JAVELIN_SPLIT_RX_PIN);
  gpio_set_function(JAVELIN_SPLIT_RX_PIN, GPIO_FUNC_PIO0);

  txConfig = pico_split_program_get_default_config(programOffset);
  sm_config_set_out_pins(&txConfig, JAVELIN_SPLIT_TX_PIN, 1);
  sm_config_set_set_pins(&txConfig, JAVELIN_SPLIT_TX_PIN, 1);
  sm_config_set_sideset_pins(&txConfig, JAVELIN_SPLIT_TX_PIN);
  sm_config_set_in_shift(&txConfig, true, true, 32);
  sm_config_set_fifo_join(&txConfig, PIO_FIFO_JOIN_TX);
  sm_config_set_out_shift(&txConfig, true, true, 32);
  sm_config_set_clkdiv_int_frac(&txConfig, 1, 0);

  rxConfig = pico_split_program_get_default_config(programOffset);
  sm_config_set_in_pins(&rxConfig, JAVELIN_SPLIT_RX_PIN);
  sm_config_set_jmp_pin(&rxConfig, JAVELIN_SPLIT_RX_PIN);
  sm_config_set_in_shift(&rxConfig, true, true, 32);
  sm_config_set_fifo_join(&rxConfig, PIO_FIFO_JOIN_RX);
  sm_config_set_out_shift(&rxConfig, true, true, 32);
  sm_config_set_clkdiv_int_frac(&rxConfig, 1, 0);

#endif

  irq_set_exclusive_handler(PIO0_IRQ_0, TxIrqHandler);
  irq_set_enabled(PIO0_IRQ_0, true);
  pio_set_irq0_source_enabled(PIO_INSTANCE, pis_interrupt0, true);

  dma2->destination = &PIO_INSTANCE->txf[TX_STATE_MACHINE_INDEX];
  constexpr PicoDmaControl sendControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::WORD,
      .incrementRead = true,
      .incrementWrite = false,
      .chainToDma = 2,
      .transferRequest = PicoDmaTransferRequest::PIO0_TX0,
      .sniffEnable = false,
  };
  dma2->control = sendControl;

  dma3->source = &PIO_INSTANCE->rxf[RX_STATE_MACHINE_INDEX];
  constexpr PicoDmaControl receiveControl = {
    .enable = true,
    .dataSize = PicoDmaControl::DataSize::WORD,
    .incrementRead = false,
    .incrementWrite = true,
    .chainToDma = 3,
#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
    .transferRequest = PicoDmaTransferRequest::PIO0_RX0,
#else
    .transferRequest = PicoDmaTransferRequest::PIO0_RX1,
#endif
    .sniffEnable = false,
  };
  dma3->control = receiveControl;
  if (!IsMaster()) {
    StartRx();
  }
}

void PicoSplit::SplitData::StartTx() {
  const PIO pio = PIO_INSTANCE;
  const int sm = TX_STATE_MACHINE_INDEX;
  const int pin = JAVELIN_SPLIT_TX_PIN;

  pio_sm_set_enabled(pio, sm, false);
  pio_sm_set_pins_with_mask(pio, sm, 0, 1 << pin);
  pio_sm_set_pindirs_with_mask(pio, sm, 1 << pin, 1 << pin);
#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
  pio_sm_init(pio, sm, programOffset + pico_split_offset_tx_start, &config);
#else
  pio_sm_init(pio, sm, programOffset + pico_split_offset_tx_start, &txConfig);
#endif

  pio_sm_set_enabled(pio, sm, true);

#if JAVELIN_SPLIT_TX_PIN != JAVELIN_SPLIT_RX_PIN
  StartRx();
#endif
}

void PicoSplit::SplitData::StartRx() {
  const PIO pio = PIO_INSTANCE;
  const int sm = RX_STATE_MACHINE_INDEX;
  const int pin = JAVELIN_SPLIT_RX_PIN;

  pio_sm_set_enabled(pio, sm, false);
  pio_sm_set_pindirs_with_mask(pio, sm, 0, 1 << pin);
#if JAVELIN_SPLIT_TX_PIN == JAVELIN_SPLIT_RX_PIN
  pio_sm_init(pio, sm, programOffset + pico_split_offset_rx_start, &config);
#else
  pio_sm_init(pio, sm, programOffset + pico_split_offset_rx_start, &rxConfig);
#endif

  ResetRxDma();
  state = State::RECEIVING;
  receiveStartTime = time_us_32();

  pio_sm_set_enabled(pio, sm, true);
}

void PicoSplit::SplitData::ResetRxDma() {
  dma3->Abort();
  dma3->destination = &rxBuffer.header;
  dma3->countTrigger = sizeof(RxBuffer) / sizeof(uint32_t);
}

void PicoSplit::SplitData::SendTxBuffer() {
  // Since Rx immediately follows Tx, set Rx dma before sending anything.
  ResetRxDma();

  const size_t wordCount = txBuffer.GetWordCount();
  txWords += wordCount;

  const size_t bitCount = 32 * wordCount;
  pio_sm_put_blocking(PIO_INSTANCE, TX_STATE_MACHINE_INDEX, bitCount - 1);
  dma2->source = &txBuffer.header;
  dma2->countTrigger = wordCount;
}

void PicoSplit::SplitData::OnReceiveFailed() {
  const uint32_t timeoutCount =
      Split::IsMaster() ? RETRY_TIMEOUT_US / MASTER_RECEIVE_TIMEOUT_US
                        : RETRY_TIMEOUT_US / SLAVE_RECEIVE_TIMEOUT_US;
  if (++retryCount > timeoutCount) {
    isConnected = false;
    updateSendData = true;
    retryCount = 0;
    metrics[SplitMetricId::RESET_COUNT]++;

    RxBuffer::OnConnectionReset();
    TxBuffer::OnConnectionReset();
    ButtonScriptManager::ExecuteScript(ButtonScriptId::PAIR_CONNECTION_UPDATE);
  }

  if (IsMaster()) {
    state = State::READY_TO_SEND;
  } else {
    StartRx();
  }
}

void PicoSplit::SplitData::OnReceiveTimeout() { OnReceiveFailed(); }

void PicoSplit::SplitData::OnReceiveSucceeded() {
  // Receiving succeeded without timeout means the previous transmit was
  // successful.
  // After receiving data, immediately start sending the data here.
  updateSendData = true;
  retryCount = 0;
  SendData();
}

bool PicoSplit::SplitData::ProcessReceive() {
  const size_t dma3Count = dma3->count;
  const size_t receivedWordCount =
      sizeof(RxBuffer) / sizeof(uint32_t) - dma3Count;

  switch (rxBuffer.Validate(receivedWordCount, metrics)) {
  case RxBufferValidateResult::CONTINUE:
    return false;

  case RxBufferValidateResult::ERROR:
    OnReceiveFailed();
    return true;

  case RxBufferValidateResult::OK:
    ++rxPacketCount;
    rxWords += rxBuffer.GetWordCount();

    if (rxBuffer.header.transferId == lastRxId) {
      metrics[SplitMetricId::REPEAT_DATA_COUNT]++;

      // Repeat of the last, don't process the data again. Resend previous data.
      SendData();
      return true;
    }

    lastRxId = rxBuffer.header.transferId;

    if (!isConnected) [[unlikely]] {
      isConnected = true;
      TxBuffer::OnConnect();
      RxBuffer::OnConnect();
      ButtonScriptManager::ExecuteScript(
          ButtonScriptId::PAIR_CONNECTION_UPDATE);
    }

    rxBuffer.Process();
    OnReceiveSucceeded();
    break;
  }

  return true;
}

void PicoSplit::SplitData::SendData() {
  dma2->WaitUntilComplete();
  StartTx();
  state = State::SENDING;

  if (updateSendData) {
    updateSendData = false;
    txBuffer.Build();
    txBuffer.header.transferId = ++txId;
  }

  SendTxBuffer();
}

void PicoSplit::SplitData::Update() {
  switch (state) {
  case State::READY_TO_SEND:
    SendData();
    break;

  case State::SENDING:
    // Do nothing, wait until TxIrqHandler happens.
    break;

  case State::RECEIVING:
    if (!ProcessReceive()) {
      const uint32_t now = time_us_32();
      const uint32_t timeSinceLastUpdate = now - receiveStartTime;
      const uint32_t receiveTimeout =
          IsMaster() ? MASTER_RECEIVE_TIMEOUT_US : SLAVE_RECEIVE_TIMEOUT_US;
      if (timeSinceLastUpdate > receiveTimeout) {
        metrics[SplitMetricId::TIMEOUT_COUNT]++;
        OnReceiveTimeout();
      }
    }
    break;
  }
}

void PicoSplit::SplitData::PrintInfo() {
  const uint32_t systemClockMhz =
      frequency_count_mhz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);

  Console::Printf("Split data: %d Mbps\n",
                  systemClockMhz / JAVELIN_SPLIT_PIO_CYCLES);
  Console::Printf("  Transmitted bytes/packets: %llu/%llu\n", 4 * txWords,
                  txIrqCount);
  Console::Printf("  Received bytes/packets: %llu/%llu\n", 4 * rxWords,
                  rxPacketCount);
  Console::Printf("  Transmit types:");
  for (size_t count : TxBuffer::txPacketTypeCounts) {
    Console::Printf(" %zu", count);
  }
  Console::Printf("\n");
  Console::Printf("  Receive types:");
  for (uint32_t count : RxBuffer::rxPacketTypeCounts) {
    Console::Printf(" %u", count);
  }
  Console::Printf("\n");
  Console::Printf("  Metrics:");
  for (size_t i = 0; i < SplitMetricId::COUNT; ++i) {
    Console::Printf(" %zu", metrics[i]);
  }
  Console::Printf("\n");
}

void __no_inline_not_in_flash_func(PicoSplit::SplitData::TxIrqHandler)() {
  if constexpr (TX_STATE_MACHINE_INDEX != RX_STATE_MACHINE_INDEX) {
    pio_sm_set_enabled(PIO_INSTANCE, TX_STATE_MACHINE_INDEX, false);
  }
  pio_interrupt_clear(PIO_INSTANCE, TX_STATE_MACHINE_INDEX);
  instance.txIrqCount++;
  instance.state = State::RECEIVING;
  instance.receiveStartTime = time_us_32();
}

//---------------------------------------------------------------------------

bool Split::IsPairConnected() { return PicoSplit::IsPairConnected(); }

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
