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

void MainReportBuilder::Reset() {
  reportBuffer.Reset();
  Mem::Clear(buffers);
  Mem::Clear(mouseBuffers);
}

void MainReportBuilder::Press(uint8_t key) {
  if (key == 0) {
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
    } else {
      buffers[1].data[MODIFIER_OFFSET] = modifiers;
      buffers[1].presenceFlags[MODIFIER_OFFSET] = 0xff;
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
  if (0xe0 <= key && key < 0xe8) {
    modifiers &= ~(1 << (key - 0xe0));
    if (maxPressIndex == 0) {
      buffers[0].data[MODIFIER_OFFSET] = modifiers;
      buffers[0].presenceFlags[MODIFIER_OFFSET] = 0xff;
    } else {
      buffers[1].data[MODIFIER_OFFSET] = modifiers;
      buffers[1].presenceFlags[MODIFIER_OFFSET] = 0xff;
    }
  } else {
    int byte = (key >> 3);
    if (key < 0xe0) {
      ++byte;
    }
    const int mask = (1 << (key & 7));

    if ((buffers[0].presenceFlags[byte] & mask) != 0 &&
        (buffers[0].data[byte] & mask) != 0) {
      buffers[1].presenceFlags[byte] |= mask;
      buffers[1].data[byte] &= ~mask;
    } else {
      buffers[0].presenceFlags[byte] |= mask;
      buffers[0].data[byte] &= ~mask;
    }
  }

  if (compatibilityMode) {
    Flush();
  }
}

bool MainReportBuilder::HasData() const {
  for (size_t i = 0; i < 8; ++i) {
    if (buffers[0].presenceFlags32[i] != 0)
      return true;
  }
  return false;
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

void MainReportBuilder::SendKeyboardPageReportIfRequired() {
  // A keyboard page report is required if there's any presence bits
  // from 0x0 - 0xa7
  if ((buffers[0].presenceFlags32[0] | buffers[0].presenceFlags32[1] |
       buffers[0].presenceFlags32[2] | buffers[0].presenceFlags32[3] |
       buffers[0].presenceFlags32[4] | buffers[0].presenceFlags[20]) == 0) {
    return;
  }

  uint8_t reportData[16];

  // The first 14 bytes match the internal buffer.
  memcpy(reportData, buffers[0].data, 14);
  reportData[14] = 0;
  reportData[15] = 0;

  // Quick reject test for array data, which resides in 0x70 - 0xa7
  if (buffers[0].presenceFlags16[7] | buffers[0].presenceFlags32[4] |
      buffers[0].presenceFlags[20]) {

    size_t offset = 14;
    for (size_t i = 0x70 / 8; i < 0xa8 / 8; ++i) {
      const uint8_t byte = buffers[0].data[i];
      if (!byte) {
        continue;
      }

      for (size_t bit = 0; bit < 8; bit++) {
        if (byte & (1 << bit)) {
          size_t logical = i * 8 + bit - 8;
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

void MainReportBuilder::SendConsumerPageReportIfRequired() {
  // Consumer page bits are 0xa8 - 0xe7
  if (((buffers[0].presenceFlags32[5] >> 8) | buffers[0].presenceFlags32[6] |
       buffers[0].presenceFlags[28]) == 0) {
    return;
  }

  SendReport(CONSUMER_PAGE_REPORT_ID, &buffers[0].data[0xa8 / 8], 8);
}

void MainReportBuilder::SendMousePageReportIfRequired() {
  if (!mouseBuffers[0].HasData()) {
    return;
  }

  SendReport(MOUSE_PAGE_REPORT_ID, (const uint8_t *)&mouseBuffers[0], 12);
}

void MainReportBuilder::SendReport(uint8_t reportId, const uint8_t *data,
                                   size_t size) {
  for (size_t i = 0; i <= repeatReportCount; ++i) {
    reportBuffer.SendReport(reportId, data, size);
  }
}

void MainReportBuilder::Flush() {
  SendKeyboardPageReportIfRequired();
  SendConsumerPageReportIfRequired();

  for (int i = 0; i < 8; ++i) {
    buffers[0].data32[i] =
        buffers[1].data32[i] |
        (~buffers[1].presenceFlags32[i] & buffers[0].data32[i]);
  }

  Mem::Copy(buffers[0].presenceFlags, buffers[1].presenceFlags,
            sizeof(buffers[0].presenceFlags));
  Mem::Clear(buffers[1]);
  maxPressIndex = 0;
}

void MainReportBuilder::FlushMouse() {
  SendMousePageReportIfRequired();

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

void Key::Press(KeyCode key) { MainReportBuilder::instance.Press(key.value); }

void Key::Release(KeyCode key) {
  MainReportBuilder::instance.Release(key.value);
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
