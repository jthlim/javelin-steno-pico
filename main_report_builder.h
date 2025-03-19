//---------------------------------------------------------------------------

#pragma once
#include "hid_report_buffer.h"
#include "javelin/mem.h"
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

class Console;

//---------------------------------------------------------------------------

struct MainReportBuilder {
public:
  void Press(uint8_t key);
  void Release(uint8_t key);

  void PressMouseButton(size_t buttonIndex);
  void ReleaseMouseButton(size_t buttonIndex);
  void MoveMouse(int dx, int dy);
  void VWheelMouse(int delta);
  void HWheelMouse(int delta);

  void FlushAllIfRequired();
  void Flush();
  void FlushMouse();
  void SendNextReport() { reportBuffer.SendNextReport(); }

  void Reset();
  size_t GetAvailableBufferCount() const {
    return reportBuffer.GetAvailableBufferCount();
  }

  void SetConfiguration(uint8_t configurationValue) {
    if (configurationValue == 0) {
      compatibilityMode = false;
      repeatReportCount = 0;
    } else {
      compatibilityMode = true;
      repeatReportCount = configurationValue - 1;
    }
  }
  void AddConsoleCommands(Console &console);
  static void GetKeyboardProtocol_Binding();

  void PrintInfo() const;

  static MainReportBuilder instance;

private:
  MainReportBuilder();

  struct Buffer {
    union {
      uint8_t data[32];
      uint32_t data32[8];
    };

    union {
      uint8_t presenceFlags[32];
      uint16_t presenceFlags16[16];
      uint32_t presenceFlags32[8];
    };
  };

  struct MouseBuffer {
    // Order is important. First 7 bytes is the report packet.
    uint32_t buttonData;
    int16_t dx, dy, vWheel, hWheel;
    uint8_t movementMask; // Bit 0 = dx,dy, Bit 1 = vWheel, Bit 2 = hWheel
    uint32_t buttonPresence;

    void Reset() { Mem::Clear(*this); }

    void SetMove(int x, int y) {
      dx = x;
      dy = y;
      movementMask |= 1;
    }

    void SetVWheel(int delta) {
      vWheel = delta;
      movementMask |= 2;
    }

    void SetHWheel(int delta) {
      hWheel = delta;
      movementMask |= 4;
    }

    bool HasMovement() const { return (movementMask & 1) != 0; }
    bool HasVWheel() const { return (movementMask & 2) != 0; }
    bool HasHWheel() const { return (movementMask & 4) != 0; }

    bool HasData() const { return (movementMask | buttonPresence) != 0; }
  };

  bool compatibilityMode = false;
  uint8_t modifiers = 0;
  uint8_t maxPressIndex = 0;
  uint8_t repeatReportCount = 0;
  int wpmTally = 0;
  Buffer buffers[2];
  MouseBuffer mouseBuffers[2];

  static constexpr size_t MAXIMUM_REPORT_DATA_SIZE = 17;
  HidReportBuffer<MAXIMUM_REPORT_DATA_SIZE> reportBuffer;

  bool HasData() const;
  bool HasMouseData() const { return mouseBuffers[0].HasData(); }
  void SendKeyboardPageReportIfRequired();
  void SendConsumerPageReportIfRequired();
  void SendMousePageReportIfRequired();

  void SendReport(uint8_t reportId, const uint8_t *data, size_t size);

  static void SetKeyboardProtocol_Binding(void *context,
                                          const char *commandLine);

  friend class SplitHidReportBuffer;
};

//---------------------------------------------------------------------------
