//---------------------------------------------------------------------------

#pragma once
#include "javelin/color.h"
#include "javelin/font/text_alignment.h"
#include "javelin/split/split.h"
#include "javelin/stroke.h"

//---------------------------------------------------------------------------

struct Font;
enum class FontId : uint32_t;
enum class St7789Command : uint8_t;

//---------------------------------------------------------------------------

#if JAVELIN_DISPLAY_DRIVER == 7789

class St7789 {
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

  static void RunConwayStep(int displayId) {
    instances[displayId].RunConwayStep();
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
  class St7789Availability : public SplitTxHandler, public SplitRxHandler {
  public:
    void operator=(bool value) { available = value; }
    bool operator!() const { return !available; }
    operator bool() const { return available; }

  private:
    bool available = true; // During startup scripts, the frame buffer may
                           // need to be written.
                           // Setting this to available will permit
                           // the slave screen to be updated until the first
                           // availability packet comes in.
    bool dirty = true;

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnDataReceived(const void *data, size_t length);
    virtual void OnReceiveConnectionReset() { available = false; }
    virtual void OnTransmitConnectionReset() { dirty = true; }
  };

  class St7789Control : public SplitTxHandler, public SplitRxHandler {
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
    struct St7789ControlTxRxData {
      bool screenOn;
      uint8_t contrast;
    };

    uint8_t dirtyFlag;
    St7789ControlTxRxData data;

    static constexpr int DIRTY_FLAG_SCREEN_ON = 1;
    static constexpr int DIRTY_FLAG_CONTRAST = 2;

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnDataReceived(const void *data, size_t length);
    virtual void OnTransmitConnectionReset();
  };

  class St7789Data : public SplitTxHandler, public SplitRxHandler {
  public:
    St7789Availability available;
    St7789Control control;
    bool dirty;
    uint16_t drawColor565 = 0xffff;
    Color drawColor = Color{0xff, 0xff, 0xff};
    size_t txRxOffset;

    void Initialize();

    // None of these will take effect until Update() is called.
    void Clear();
    void DrawLine(int x0, int y0, int x1, int y1);
    void DrawRect(int left, int top, int right, int bottom);
    void DrawBitmapImage(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawLuminanceImage(int x, int y, int width, int height,
                            const uint8_t *data);
    void DrawRgb332Image(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawRgb565Image(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawRgb888Image(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawAlpha8Image(int x, int y, int width, int height,
                         const uint8_t *data);
    void DrawArgb1555Image(int x, int y, int width, int height,
                           const uint8_t *data);
    void DrawRgba8888Image(int x, int y, int width, int height,
                           const uint8_t *data);
    void DrawLuminanceRange(int x, int y, int width, int height,
                            const uint8_t *data, int min, int max);
    void DrawText(int x, int y, const Font *font, TextAlignment alignment,
                  const char *text);
    void SetPixel(uint32_t x, uint32_t y);

    void RunConwayStep();

    void DrawPaperTape(const StenoStroke *strokes, size_t length);
    void DrawStenoLayout(StenoStroke stroke);

    void Update();

  private:
    struct RunConwayStepThreadData;

    union {
      uint8_t buffer8[2 * JAVELIN_DISPLAY_SCREEN_WIDTH *
                      JAVELIN_DISPLAY_SCREEN_HEIGHT];
      uint16_t buffer16[JAVELIN_DISPLAY_SCREEN_WIDTH *
                        JAVELIN_DISPLAY_SCREEN_HEIGHT];
      uint32_t buffer32[JAVELIN_DISPLAY_SCREEN_WIDTH *
                        JAVELIN_DISPLAY_SCREEN_HEIGHT / 2];
    };

    void SendScreenData() const;

    virtual void UpdateBuffer(TxBuffer &buffer);
    virtual void OnTransmitConnectionReset() { dirty = true; }
    virtual void OnDataReceived(const void *data, size_t length);
  };

  static void SendCommand(St7789Command command, const void *data = nullptr,
                          size_t length = 0);
  static void SetLR(uint32_t left, uint32_t right);
  static void SetTB(uint32_t top, uint32_t bottom);

#if JAVELIN_SPLIT
  static St7789Data &GetInstance() {
    if (Split::IsMaster()) {
      return instances[0];
    } else {
      return instances[1];
    }
  }
  static St7789Data instances[2];
#else
  static St7789Data &GetInstance() { return instance[0]; }
  static St7789Data instances[1];
#endif

  friend class Display;
};

#else

class St7789 {
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
