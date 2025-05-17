//---------------------------------------------------------------------------

#pragma once
#include "javelin/font/text_alignment.h"
#include "javelin/split/split.h"
#include "javelin/stroke.h"

//---------------------------------------------------------------------------

struct Font;
enum class FontId : uint32_t;

//---------------------------------------------------------------------------

#if JAVELIN_DISPLAY_DRIVER == 1306

class Ssd1306 {
public:
  static void Initialize() { GetInstance().Initialize(); }
  static void PrintInfo();

  static void Update() { GetInstance().Update(); }

  static void DrawPaperTape(int displayId, const StenoStroke *strokes,
                            size_t length) {
    instances[displayId].DrawPaperTape(strokes, length);
  }
  static void DrawStenoLayout(int displayId, StenoStroke stroke) {
    instances[displayId].DrawStenoLayout(stroke);
  }
  static void DrawText(int displayId, int x, int y, const Font *font,
                       TextAlignment alignment, const char *text) {
    instances[displayId].DrawText(x, y, font, alignment, text);
  }

#if JAVELIN_SPLIT
  static void RegisterMasterHandlers() {
    Split::RegisterRxHandler(SplitHandlerId::DISPLAY_AVAILABLE,
                             &instances[1].available);
    Split::RegisterTxHandler(&instances[1]);
    Split::RegisterTxHandler(&instances[1].control);
  }
  static void RegisterSlaveHandlers() {
    Split::RegisterTxHandler(&instances[1].available);
    Split::RegisterRxHandler(SplitHandlerId::DISPLAY_DATA, &instances[1]);
    Split::RegisterRxHandler(SplitHandlerId::DISPLAY_CONTROL,
                             &instances[1].control);
  }
#else
  static void RegisterMasterHandlers() {}
  static void RegisterSlaveHandlers() {}
#endif

private:
  class Ssd1306Availability : public SplitTxHandler, public SplitRxHandler {
  public:
    void operator=(bool value) { available = value; }
    bool operator!() const { return !available; }
    operator bool() const { return available; }

  private:
    bool available = true; // During startup scripts, the frame buffer may
                           // need to be written.
                           // Setting this to available will permit
                           // the slave OLED to be updated until the first
                           // availability packet comes in.
    bool dirty = true;

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnDataReceived(const void *data, size_t length);
    virtual void OnReceiveConnectionReset() { available = false; }
    virtual void OnTransmitConnectionReset() { dirty = true; }
  };

  class Ssd1306Control : public SplitTxHandler, public SplitRxHandler {
  public:
    void Update();
    void SetScreenOn(bool on) {
      if (on != data.screenOn) {
        data.screenOn = on;
        dirtyFlag |= DIRTY_FLAG_SCREEN_ON;
      }
    }
    void SetContrast(uint8_t value) {
      if (value != data.contrast) {
        data.contrast = value;
        dirtyFlag |= DIRTY_FLAG_CONTRAST;
      }
    }

  private:
    struct Ssd1306ControlTxRxData {
      bool screenOn;
      uint8_t contrast;
    };

    uint8_t dirtyFlag;
    Ssd1306ControlTxRxData data;

    static constexpr int DIRTY_FLAG_SCREEN_ON = 1;
    static constexpr int DIRTY_FLAG_CONTRAST = 2;

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnDataReceived(const void *data, size_t length);
    virtual void OnTransmitConnectionReset();
  };

  class Ssd1306Data : public SplitTxHandler, public SplitRxHandler {
  public:
    Ssd1306Availability available;
    Ssd1306Control control;
    bool dirty;
    bool drawColor = true;

    void Initialize();

    // None of these will take effect until Update() is called.
    void Clear();
    void DrawLine(int x0, int y0, int x1, int y1);
    void DrawRect(int left, int top, int right, int bottom);
    void DrawBitmapImage(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawLuminanceRange(int x, int y, int width, int height,
                            const uint8_t *data, int min, int max);
    void DrawText(int x, int y, const Font *font, TextAlignment alignment,
                  const char *text);
    void SetPixel(uint32_t x, uint32_t y);

    void DrawPaperTape(const StenoStroke *strokes, size_t length);
    void DrawStenoLayout(StenoStroke stroke);

    void Update();

  private:
    union {
      uint8_t buffer8[JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 8];
      uint32_t buffer32[JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 32];
    };

    bool InitializeSsd1306();

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnTransmitConnectionReset() { dirty = true; }
    virtual void OnDataReceived(const void *data, size_t length);
  };

  static uint16_t dmaBuffer[JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 8 + 1];

  static bool IsI2cTxReady();
  static void WaitForI2cTxReady();

  static void SendCommandListDma(const uint8_t *commands, size_t length);
  static bool SendCommandList(const uint8_t *commands, size_t length);
  static bool SendCommand(uint8_t command);
  static void SendDmaBuffer(size_t count);

#if JAVELIN_SPLIT
  static Ssd1306Data &GetInstance() {
    if (Split::IsMaster()) {
      return instances[0];
    } else {
      return instances[1];
    }
  }
  static Ssd1306Data instances[2];
#else
  static Ssd1306Data &GetInstance() { return instance[0]; }
  static Ssd1306Data instances[1];
#endif

  friend class Display;
};

#else

class Ssd1306 {
public:
  static void Initialize() {}
  static void PrintInfo() {}
  static void DrawPaperTape() {}

  static void Update() {}

  static void RegisterMasterHandlers() {}
  static void RegisterSlaveHandlers() {}
};

#endif

//---------------------------------------------------------------------------
