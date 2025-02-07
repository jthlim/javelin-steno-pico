//---------------------------------------------------------------------------

#include JAVELIN_BOARD_CONFIG

#include "console_report_buffer.h"
#include "javelin/button_script_manager.h"
#include "javelin/console_input_buffer.h"
#include "javelin/keyboard_led_status.h"
#include "javelin/split/split_console.h"
#include "javelin/split/split_serial_buffer.h"
#include "javelin/split/split_usb_status.h"
#include "javelin/split/split_version.h"
#include "javelin/static_allocate.h"
#include "main_report_builder.h"
#include "main_task.h"
#include "pinnacle.h"
#include "plover_hid_report_buffer.h"
#include "rp2040_button_state.h"
#include "rp2040_crc.h"
#include "rp2040_encoder_state.h"
#include "rp2040_split.h"
#include "rp2040_ws2812.h"
#include "split_hid_report_buffer.h"
#include "ssd1306.h"
#include "st7789.h"
#include "usb_descriptors.h"

#include <hardware/watchdog.h>
#include <pico/time.h>
#include <tusb.h>

#if defined(CONFIG_EXTRA_SOURCE)
#include CONFIG_EXTRA_SOURCE
#endif

//---------------------------------------------------------------------------

void InitJavelinMaster();
void InitJavelinSlave();
void FlushBuffers();
void InitMulticore();

//---------------------------------------------------------------------------

#if JAVELIN_USE_WATCHDOG
extern uint32_t watchdogData[8];
#endif

//---------------------------------------------------------------------------
// Device callbacks
//---------------------------------------------------------------------------

// Invoked when device is mounted
extern "C" void tud_mount_cb(void) {
  ConsoleReportBuffer::instance.Reset();
  PloverHidReportBuffer::instance.Reset();
  MainReportBuilder::instance.Reset();
  SplitUsbStatus::instance.OnMount();
  SplitUsbStatus::instance.SetPowered(true);
  ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
}

// Invoked when device is unmounted
extern "C" void tud_umount_cb(void) {
  SplitUsbStatus::instance.OnUnmount();
  SplitUsbStatus::instance.SetPowered(false);
  ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
}

// Invoked when usb bus is suspended
// remoteWakeupEnabled: if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
extern "C" void tud_suspend_cb(bool remoteWakeupEnabled) {
  // set_sys_clock_khz(25000, true);
  SplitUsbStatus::instance.OnSuspend();
  ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
}

// Invoked when usb bus is resumed
extern "C" void tud_resume_cb(void) {
  // set_sys_clock_khz(125000, true);
  SplitUsbStatus::instance.OnResume();
  ButtonScriptManager::ExecuteScript(ButtonScriptId::CONNECTION_UPDATE);
}

//---------------------------------------------------------------------------
// USB HID
//---------------------------------------------------------------------------

// Invoked when received SET_PROTOCOL request
// protocol is either HID_PROTOCOL_BOOT (0) or HID_PROTOCOL_REPORT (1)
extern "C" void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
extern "C" void tud_hid_report_complete_cb(uint8_t instance,
                                           const uint8_t *report,
                                           uint16_t length) {
  switch (instance) {
  case ITF_NUM_KEYBOARD:
    MainReportBuilder::instance.SendNextReport();
    break;
  case ITF_NUM_PLOVER_HID:
    PloverHidReportBuffer::instance.SendNextReport();
    break;
  case ITF_NUM_CONSOLE:
    ConsoleReportBuffer::instance.SendNextReport();
    break;
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t reportId,
                                          hid_report_type_t report_type,
                                          uint8_t *buffer, uint16_t reqlen) {
  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t reportId,
                                      hid_report_type_t reportType,
                                      const uint8_t *buffer,
                                      uint16_t bufferSize) {
  switch (instance) {
  case ITF_NUM_KEYBOARD:
    if (reportType != HID_REPORT_TYPE_OUTPUT ||
        reportId != KEYBOARD_PAGE_REPORT_ID || bufferSize < 1) {
      return;
    }
    SplitUsbStatus::instance.SetKeyboardLedStatus(
        *(KeyboardLedStatus *)&buffer[0]);
    break;

  case ITF_NUM_CONSOLE:
    ConsoleInputBuffer::Add(buffer, bufferSize, ConnectionId::USB);
    break;
  }
}

//---------------------------------------------------------------------------

static void cdc_task() {
  for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++) {
    if (tud_cdc_n_available(itf)) {
      uint8_t buf[64];
      const uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
      // Do nothing with it.
    }
  }
}

//---------------------------------------------------------------------------

// By default, the system will run as fast as possible with 100us pauses.
//
// This interrupt handling is a fallback to catch key and encoder changes
// while long-running processes occur.

static void IrqHandler() {
  hw_clear_bits(&timer_hw->intr, 1);
  Rp2040ButtonState::Update();
  Rp2040EncoderState::UpdateNoScriptCall();
  timer_hw->alarm[0] = timer_hw->timerawl + 3'000;
}

static void InitializeInterruptUpdate() {
  irq_set_exclusive_handler(TIMER_IRQ_0, IrqHandler);
  irq_set_enabled(TIMER_IRQ_0, true);
}

static void StartInterruptUpdate() {
  hw_set_bits(&timer_hw->inte, 1);
  timer_hw->alarm[0] = timer_hw->timerawl + 3'000;
}

static void StopInterruptUpdate() { hw_clear_bits(&timer_hw->inte, 1); }

//---------------------------------------------------------------------------

void DoMasterRunLoop() {
#if JAVELIN_USE_WATCHDOG
  watchdog_enable(1000, true);
#endif

  while (1) {
    Rp2040ButtonState::Update();
    StartInterruptUpdate();

    tud_task(); // tinyusb device task

    MasterTask::container->Update();
    Rp2040Split::Update();
    cdc_task();

    FlushBuffers();
    ConsoleInputBuffer::Process();
    Ws2812::Update();
    Ssd1306::Update();
    St7789::Update();

#if JAVELIN_USE_WATCHDOG
    watchdog_update();
#endif
    StopInterruptUpdate();
    sleep_us(100);
  }
}

void DoSlaveRunLoop() {
#if JAVELIN_USE_WATCHDOG
  watchdog_enable(1000, true);
#endif

  while (1) {
    Rp2040ButtonState::Update();
    StartInterruptUpdate();

    tud_task(); // tinyusb device task
    SlaveTask::container->Update();
    Rp2040Split::Update();
    cdc_task();

    SplitHidReportBuffer::Update();
    FlushBuffers();
    ConsoleInputBuffer::Process();
    Ws2812::Update();
    Ssd1306::Update();
    St7789::Update();
    SplitConsole::Process();

#if JAVELIN_USE_WATCHDOG
    watchdog_update();
#endif
    StopInterruptUpdate();
    sleep_us(100);
  }
}

int main(void) {
#if JAVELIN_USE_WATCHDOG
  for (int i = 0; i < 8; ++i) {
    watchdogData[i] = watchdog_hw->scratch[i];
  }
#endif

#if JAVELIN_THREADS
  InitMulticore();
#endif
  Rp2040Crc::Initialize();
  Ws2812::Initialize();
  Rp2040Split::Initialize();
  Pinnacle::Initialize();
  Ssd1306::Initialize();
  St7789::Initialize();

#if defined(JAVELIN_PRE_BUTTON_STATE_INITIALIZE)
  JAVELIN_PRE_BUTTON_STATE_INITIALIZE
#endif

  Rp2040ButtonState::Initialize();
  Rp2040EncoderState::Initialize();

  InitializeInterruptUpdate();

  if (Split::IsMaster()) {
    new (MasterTask::container) MasterTask;

    Split::RegisterTxHandler(&MasterTask::container.value);
    Split::RegisterRxHandler(SplitHandlerId::KEY_STATE,
                             &MasterTask::container.value);
    ConsoleInputBuffer::RegisterRxHandler();
    Ws2812::RegisterTxHandler();
    SplitHidReportBuffer::RegisterMasterHandlers();
    SplitSerialBuffer::RegisterTxHandler();
    SplitVersion::RegisterRxHandler();
    Pinnacle::RegisterRxHandler();
    Ssd1306::RegisterMasterHandlers();
    St7789::RegisterMasterHandlers();
    SplitUsbStatus::RegisterHandlers();
    SplitConsole::RegisterHandlers();
    Rp2040EncoderState::RegisterRxHandler();

    InitJavelinMaster();
    ButtonScriptManager::Initialize(SCRIPT_BYTE_CODE);
    tusb_init();

    DoMasterRunLoop();
  } else {
    new (SlaveTask::container) SlaveTask;

    Split::RegisterTxHandler(&SlaveTask::container.value);
    Split::RegisterRxHandler(SplitHandlerId::KEY_STATE,
                             &SlaveTask::container.value);
    ConsoleInputBuffer::RegisterTxHandler();
    Ws2812::RegisterRxHandler();
    SplitHidReportBuffer::RegisterSlaveHandlers();
    SplitSerialBuffer::RegisterRxHandler();
    SplitVersion::RegisterTxHandler();
    Pinnacle::RegisterTxHandler();
    Ssd1306::RegisterSlaveHandlers();
    St7789::RegisterSlaveHandlers();
    SplitUsbStatus::RegisterHandlers();
    SplitConsole::RegisterHandlers();
    Rp2040EncoderState::RegisterTxHandler();

    InitJavelinSlave();
    ButtonScriptManager::Initialize(SCRIPT_BYTE_CODE);

    tusb_init();
    DoSlaveRunLoop();
  }

  return 0;
}

//---------------------------------------------------------------------------
