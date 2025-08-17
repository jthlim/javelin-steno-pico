//---------------------------------------------------------------------------

#pragma once
#include JAVELIN_BOARD_CONFIG
#include "javelin/processor/passthrough.h"
#include "javelin/static_allocate.h"
#include "javelin/timer_manager.h"

//---------------------------------------------------------------------------

#if JAVELIN_DISPLAY_DRIVER

enum class AutoDraw : uint8_t {
  NONE,
  PAPER_TAPE,
  STENO_LAYOUT,
  WPM,
  STROKES,
};

class StenoStrokeCapture final : public StenoPassthrough, private TimerHandler {
private:
  using super = StenoPassthrough;

public:
  StenoStrokeCapture(StenoProcessorElement *next) : super(next) {}

  void Process(const StenoKeyState &value, StenoAction action) override;

  void Update(bool onStrokeInput);

  void SetAutoDraw(int displayId, AutoDraw autoDrawId);

  static JavelinStaticAllocate<StenoStrokeCapture> container;

private:
  static constexpr size_t MAXIMUM_STROKE_COUNT = 32;
  size_t strokeCount = 0;
#if JAVELIN_SPLIT
  AutoDraw autoDraw[2];
  int lastWpm[2] = {-1, -1};
#else
  AutoDraw autoDraw[1];
  int lastWpm[1] = {-1};
#endif
  StenoStroke strokes[MAXIMUM_STROKE_COUNT];

  void DrawWpm(int displayId);
  void DrawStrokes(int displayId);
  void SetWpmTimer(bool enable);

  // Override from TimerHandler
  void Run(intptr_t id) final { WpmTimerHandler(); }
  void WpmTimerHandler();
};

#endif

//---------------------------------------------------------------------------
