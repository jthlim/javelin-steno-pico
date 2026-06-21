//---------------------------------------------------------------------------

#include "main_report_builder.h"
#include "javelin/console.h"
#include "javelin/hal/mouse.h"
#include "javelin/key.h"
#include "javelin/mem.h"
#include "javelin/str.h"
#include "javelin/unicode.h"
#include "javelin/wpm_tracker.h"
#include "usb_descriptors.h"

#include <string.h>

//---------------------------------------------------------------------------

MainReportBuilder MainReportBuilder::instance;

//---------------------------------------------------------------------------

const size_t MODIFIER_OFFSET = 0;

MainReportBuilder::MainReportBuilder() : reportBuffer(ITF_NUM_KEYBOARD) {}

//---------------------------------------------------------------------------

inline void MainReportBuilder::Buffer::Reset() { Mem::Clear(*this); }

//---------------------------------------------------------------------------

// clang-format off
// TODO: July 2027, Cleanup once full consumer page support is stabilized.
static const uint16_t KEY_CODE_TO_CONSUMER_CODE[] = {
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, // Play->Stop
  0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, // Eject->Slow Tracking
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, // Frame Forward->Search Mark Backward
  0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, // 
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, // 
  0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, // 
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, // 
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0x6f, 0x70, // ..., Brightness up/down
};
// clang-format on

//---------------------------------------------------------------------------

void MainReportBuilder::Reset() {
  reportBuffer.Reset();
  consumerPageBuffer.Reset();
  Mem::Clear(buffers);
  Mem::Clear(mouseBuffers);
}

void MainReportBuilder::Press(uint8_t key) {
  if (key == 0) {
    return;
  }

  // TODO: July 2027, Cleanup once full consumer page support is stabilized.
  if (key >= 0xa0 && key < 0xe0) [[unlikely]] {
    PressConsumerPageCode(KEY_CODE_TO_CONSUMER_CODE[key - 0xa0]);
    return;
  }

  if (key == KeyCode::BACKSPACE) {
    --wpmTally;
  } else if (KeyCode(key).IsVisible()) {
    ++wpmTally;
  }

  if (0xe0 <= key && key < 0xe8) {
    modifiers |= (1 << (key - 0xe0));
    if (maxPressIndex == 0) {
      buffers[0].data[MODIFIER_OFFSET] = modifiers;
      buffers[0].presenceFlags[MODIFIER_OFFSET] = 0xff;
      buffers[0].presence = 0xff;
    } else {
      buffers[1].data[MODIFIER_OFFSET] = modifiers;
      buffers[1].presenceFlags[MODIFIER_OFFSET] = 0xff;
      buffers[1].presence = 0xff;
    }
  } else {

    int byte = (key >> 3);
    if (key < 0xe0) {
      ++byte;
    }
    const int mask = (1 << (key & 7));

    if (key <= maxPressIndex ||
        (maxPressIndex != 0 && buffers[0].data[MODIFIER_OFFSET] != modifiers) ||
        (buffers[0].presenceFlags[byte] & mask)) {
      Flush();
    }

    if (buffers[0].presenceFlags[byte] & mask) {
      Flush();
    }

    buffers[0].data[MODIFIER_OFFSET] = modifiers;
    buffers[0].presenceFlags[MODIFIER_OFFSET] = 1;
    buffers[0].presence = 1;
    buffers[0].data[byte] |= mask;
    buffers[0].presenceFlags[byte] |= mask;
    if (key > maxPressIndex) {
      maxPressIndex = key;
    }
  }

  if (compatibilityMode) {
    Flush();
  }
}

void MainReportBuilder::Release(uint8_t key) {
  if (key == 0) {
    return;
  }

  // TODO: July 2027, Cleanup once full consumer page support is stabilized.
  if (key >= 0xa0 && key < 0xe0) [[unlikely]] {
    ReleaseConsumerPageCode(KEY_CODE_TO_CONSUMER_CODE[key - 0xa0]);
    return;
  }

  if (0xe0 <= key && key < 0xe8) {
    modifiers &= ~(1 << (key - 0xe0));
    if (maxPressIndex == 0) {
      buffers[0].data[MODIFIER_OFFSET] = modifiers;
      buffers[0].presenceFlags[MODIFIER_OFFSET] = 0xff;
      buffers[0].presence = 0xff;
    } else {
      buffers[1].data[MODIFIER_OFFSET] = modifiers;
      buffers[1].presenceFlags[MODIFIER_OFFSET] = 0xff;
      buffers[1].presence = 0xff;
    }
  } else {
    int byte = (key >> 3);
    if (key < 0xe0) {
      ++byte;
    }
    const int mask = (1 << (key & 7));

    if ((buffers[0].presenceFlags[byte] & mask) != 0 &&
        (buffers[0].data[byte] & mask) != 0) {
      buffers[1].presence = mask;
      buffers[1].presenceFlags[byte] |= mask;
      buffers[1].data[byte] &= ~mask;
    } else {
      buffers[0].presence = mask;
      buffers[0].presenceFlags[byte] |= mask;
      buffers[0].data[byte] &= ~mask;
    }
  }

  if (compatibilityMode) {
    Flush();
  }
}

bool MainReportBuilder::ConsumerPageBuffer::AddScanCode(uint32_t code) {
  if (scanCodes.Contains(code)) {
    return false;
  }
  if (scanCodes.IsFull()) {
    return false;
  }

  scanCodes.Add(code);
  return true;
}

bool MainReportBuilder::ConsumerPageBuffer::RemoveScanCode(uint32_t code) {
  const size_t index = scanCodes.FindIndexOf(code);
  if (index == -1) {
    return false;
  }

  scanCodes.RemoveIndex(index);
  return true;
}

void MainReportBuilder::PressConsumerPageCode(uint32_t code) {
  if (consumerPageBuffer.AddScanCode(code)) {
    Flush();
    SendConsumerPageReport();
  }
}

void MainReportBuilder::ReleaseConsumerPageCode(uint32_t code) {
  if (consumerPageBuffer.RemoveScanCode(code)) {
    Flush();
    SendConsumerPageReport();
  }
}

void MainReportBuilder::FlushAllIfRequired() {
  if (HasData()) {
    Flush();
    if (HasData()) {
      Flush();
    }
  }
  if (HasMouseData()) {
    FlushMouse();
    if (HasMouseData()) {
      FlushMouse();
    }
  }
  if (const int value = wpmTally; value) {
    wpmTally = 0;
    WpmTracker::instance.Tally(value);
  }
}

void MainReportBuilder::SendKeyboardPageReport() {
  uint8_t reportData[16];

  // The first 14 bytes match the internal buffer.
  memcpy(reportData, buffers[0].data, 14);
  reportData[14] = 0;
  reportData[15] = 0;

  // Quick reject test for array data, which resides in 0x70 - 0xa7
  if (buffers[0].presenceFlags16[7] | buffers[0].presenceFlags32[4] |
      buffers[0].presenceFlags32[5] | buffers[0].presenceFlags32[6] |
      buffers[0].presenceFlags32[7]) {

    size_t offset = 14;
    for (size_t i = 0x70 / 8; i < 0x100 / 8; ++i) {
      const uint8_t byte = buffers[0].data[i];
      if (!byte) {
        continue;
      }

      for (size_t bit = 0; bit < 8; bit++) {
        if (byte & (1 << bit)) {
          size_t logical = i * 8 + bit;
          if (logical < 0xe8) {
            logical -= 8;
          }
          reportData[offset++] = logical;
          if (offset == 16) {
            goto done;
          }
        }
      }
    }
  }
done:
  SendReport(KEYBOARD_PAGE_REPORT_ID, reportData, sizeof(reportData));
}

void MainReportBuilder::ConsumerPageBuffer::BuildConsumerPageReport(
    uint8_t (&reportData)[16]) const {
  uint8_t *p = &reportData[0];
  for (const uint16_t scanCode : scanCodes) {
    *p++ = uint8_t(scanCode);
    *p++ = uint8_t(scanCode >> 8);
  }
}

void MainReportBuilder::SendConsumerPageReport() {
  uint8_t reportData[16] = {};
  consumerPageBuffer.BuildConsumerPageReport(reportData);
  SendReport(CONSUMER_PAGE_REPORT_ID, reportData, 16);
}

void MainReportBuilder::SendMousePageReport() {
  reportBuffer.SendReport(MOUSE_PAGE_REPORT_ID,
                          (const uint8_t *)&mouseBuffers[0], 12);
}

void MainReportBuilder::SendReport(uint8_t reportId, const uint8_t *data,
                                   size_t size) {
  for (size_t i = 0; i <= repeatReportCount; ++i) {
    reportBuffer.SendReport(reportId, data, size);
  }
}

void MainReportBuilder::Flush() {
  SendKeyboardPageReport();

  for (int i = 0; i < 8; ++i) {
    buffers[0].data32[i] =
        buffers[1].data32[i] |
        (~buffers[1].presenceFlags32[i] & buffers[0].data32[i]);
  }

  Mem::Copy(buffers[0].presenceFlags32, buffers[1].presenceFlags32,
            sizeof(buffers[0].presenceFlags32));
  buffers[0].presence = buffers[1].presence;
  Mem::Clear(buffers[1]);
  maxPressIndex = 0;
}

void MainReportBuilder::FlushMouse() {
  SendMousePageReport();

  mouseBuffers[0].buttonData =
      mouseBuffers[1].buttonData |
      (~mouseBuffers[1].buttonPresence & mouseBuffers[0].buttonData);
  mouseBuffers[0].buttonPresence = mouseBuffers[1].buttonPresence;

  mouseBuffers[0].movementMask = mouseBuffers[1].movementMask;
  mouseBuffers[0].dx = mouseBuffers[1].dx;
  mouseBuffers[0].dy = mouseBuffers[1].dy;
  mouseBuffers[0].vWheel = mouseBuffers[1].vWheel;
  mouseBuffers[0].hWheel = mouseBuffers[1].hWheel;

  mouseBuffers[1].Reset();
}

//---------------------------------------------------------------------------

void MainReportBuilder::PressMouseButton(size_t buttonIndex) {
  const size_t buttonMask = 1 << buttonIndex;
  const size_t previousButtonMask = buttonMask - 1;
  if (mouseBuffers[0].buttonPresence & (buttonMask | previousButtonMask)) {
    FlushMouse();
  }
  if (mouseBuffers[0].buttonPresence & previousButtonMask) {
    mouseBuffers[1].buttonPresence |= buttonMask;
    mouseBuffers[1].buttonData |= buttonMask;
  } else {
    mouseBuffers[0].buttonPresence |= buttonMask;
    mouseBuffers[0].buttonData |= buttonMask;
  }
}

void MainReportBuilder::ReleaseMouseButton(size_t buttonIndex) {
  const size_t buttonMask = 1 << buttonIndex;
  if (mouseBuffers[0].buttonPresence & mouseBuffers[0].buttonData &
      buttonMask) {
    mouseBuffers[1].buttonPresence |= buttonMask;
    mouseBuffers[1].buttonData &= ~buttonMask;
  } else {
    mouseBuffers[0].buttonPresence |= buttonMask;
    mouseBuffers[0].buttonData &= ~buttonMask;
  }
}

void MainReportBuilder::MoveMouse(int dx, int dy) {
  if (!mouseBuffers[0].HasMovement()) {
    mouseBuffers[0].SetMove(dx, dy);
    return;
  }

  if (mouseBuffers[1].HasMovement()) {
    FlushMouse();
  }

  mouseBuffers[1].SetMove(dx, dy);
}

void MainReportBuilder::VWheelMouse(int delta) {
  if (!mouseBuffers[0].HasVWheel()) {
    mouseBuffers[0].SetVWheel(delta);
    return;
  }

  if (mouseBuffers[1].HasVWheel()) {
    FlushMouse();
  }

  mouseBuffers[1].SetVWheel(delta);
}

void MainReportBuilder::HWheelMouse(int delta) {
  if (!mouseBuffers[0].HasHWheel()) {
    mouseBuffers[0].SetHWheel(delta);
    return;
  }

  if (mouseBuffers[1].HasHWheel()) {
    FlushMouse();
  }

  mouseBuffers[1].SetHWheel(delta);
}

//---------------------------------------------------------------------------

void MainReportBuilder::SetKeyboardProtocol_Binding(void *context,
                                                    const char *commandLine) {
  const char *keyboardProtocol = strchr(commandLine, ' ');
  if (!keyboardProtocol) {
    Console::Printf("ERR No keyboard protocol specified\n\n");
    return;
  }

  ++keyboardProtocol;
  int delay = 0;
  if (Unicode::IsAsciiDigit(*keyboardProtocol)) {
    const char *p = Str::ParseInteger(&delay, keyboardProtocol);
    if (p) {
      keyboardProtocol = p + 1;
    }
  }

  MainReportBuilder *instance = (MainReportBuilder *)context;
  if (Str::Eq(keyboardProtocol, "default")) {
    instance->compatibilityMode = false;
    instance->repeatReportCount = 0;
  } else if (Str::Eq(keyboardProtocol, "compatibility")) {
    instance->compatibilityMode = true;
    instance->repeatReportCount = delay;
  } else {
    Console::Printf("ERR Unable to set keyboard protocol: \"%s\"\n\n",
                    keyboardProtocol);
    return;
  }

  Console::SendOk();
}

void MainReportBuilder::AddConsoleCommands(Console &console) {
  console.RegisterCommand("set_keyboard_protocol",
                          "Sets the current keyboard protocol "
                          "[\"default\", \"compatibility\"]",
                          &SetKeyboardProtocol_Binding, this);
}

void MainReportBuilder::GetKeyboardProtocol_Binding() {
  Console::Printf("%d %s\n\n", instance.repeatReportCount,
                  instance.compatibilityMode ? "compatibility" : "default");
}

//---------------------------------------------------------------------------

void MainReportBuilder::PrintInfo() const {
  Console::Printf("Keyboard protocol: %s\n",
                  compatibilityMode ? "compatibility" : "default");
}

//---------------------------------------------------------------------------

void Key::Press(KeyCode key) {
  const uint32_t category = key.value >> 16;
  switch (category) {
  case 0:
    MainReportBuilder::instance.Press(key.value);
    break;
  case 1:
    MainReportBuilder::instance.PressConsumerPageCode(key.value & 0xffff);
    break;
  }
}

void Key::Release(KeyCode key) {
  const uint32_t category = key.value >> 16;
  switch (category) {
  case 0:
    MainReportBuilder::instance.Release(key.value);
    break;
  case 1:
    MainReportBuilder::instance.ReleaseConsumerPageCode(key.value & 0xffff);
    break;
  }
}

void Key::Flush() { MainReportBuilder::instance.Flush(); }

//---------------------------------------------------------------------------

void Mouse::PressButton(size_t index) {
  MainReportBuilder::instance.PressMouseButton(index);
}

void Mouse::ReleaseButton(size_t index) {
  MainReportBuilder::instance.ReleaseMouseButton(index);
}

void Mouse::Move(int dx, int dy) {
  MainReportBuilder::instance.MoveMouse(dx, dy);
}

void Mouse::VWheel(int delta) {
  MainReportBuilder::instance.VWheelMouse(delta);
}

void Mouse::HWheel(int delta) {
  MainReportBuilder::instance.HWheelMouse(delta);
}

//---------------------------------------------------------------------------
