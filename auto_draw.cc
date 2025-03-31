//---------------------------------------------------------------------------

#include "auto_draw.h"
#include "javelin/clock.h"
#include "javelin/console.h"
#include "javelin/font/monochrome/font.h"
#include "javelin/hal/display.h"
#include "javelin/str.h"
#include "javelin/timer_manager.h"
#include "javelin/wpm_tracker.h"
#include "ssd1306.h"
#include "st7789.h"

//---------------------------------------------------------------------------

#if JAVELIN_DISPLAY_DRIVER

#if JAVELIN_DISPLAY_DRIVER == 1306
#define DisplayDriver Ssd1306
#elif JAVELIN_DISPLAY_DRIVER == 7789
#define DisplayDriver St7789
#endif

//---------------------------------------------------------------------------

JavelinStaticAllocate<StenoStrokeCapture> StenoStrokeCapture::container;

//---------------------------------------------------------------------------

void StenoStrokeCapture::Process(const StenoKeyState &value,
                                 StenoAction action) {
  super::Process(value, action);
  if (action == StenoAction::TRIGGER) {
    if (strokeCount < MAXIMUM_STROKE_COUNT) {
      strokes[strokeCount] = value.ToStroke();
    } else {
      memmove(&strokes[0], &strokes[1],
              sizeof(StenoStroke) * (MAXIMUM_STROKE_COUNT - 1));
      strokes[MAXIMUM_STROKE_COUNT - 1] = value.ToStroke();
    }
    ++strokeCount;
    Update(true);
  }
}

void StenoStrokeCapture::Update(bool onStrokeInput) {
#if JAVELIN_SPLIT
  for (int displayId = 0; displayId < 2; ++displayId) {
#else
  const int displayId = 0;
  {
#endif
    switch (autoDraw[displayId]) {
    case AutoDraw::NONE:
      // Do nothing
      break;
    case AutoDraw::PAPER_TAPE:
      DisplayDriver::DrawPaperTape(displayId, strokes,
                                   strokeCount > MAXIMUM_STROKE_COUNT
                                       ? MAXIMUM_STROKE_COUNT
                                       : strokeCount);
      break;
    case AutoDraw::STENO_LAYOUT:
      DisplayDriver::DrawStenoLayout(displayId,
                                     strokeCount == 0 ? StenoStroke(0)
                                     : strokeCount < MAXIMUM_STROKE_COUNT
                                         ? strokes[strokeCount - 1]
                                         : strokes[MAXIMUM_STROKE_COUNT - 1]);
      break;
    case AutoDraw::WPM:
      if (!onStrokeInput) {
        DrawWpm(displayId);
      }
      break;

    case AutoDraw::STROKES:
      DrawStrokes(displayId);
      break;
    }
  }
}

void StenoStrokeCapture::SetAutoDraw(int displayId, AutoDraw autoDrawId) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  if (autoDrawId != AutoDraw::WPM) {
    lastWpm[displayId] = -1;
  }
  autoDraw[displayId] = autoDrawId;
  Update(false);
#if JAVELIN_SPLIT
  SetWpmTimer(autoDraw[0] == AutoDraw::WPM || autoDraw[1] == AutoDraw::WPM);
#else
  SetWpmTimer(autoDraw[0] == AutoDraw::WPM);
#endif
}

void StenoStrokeCapture::SetWpmTimer(bool enable) {
  const intptr_t timerId = -('A' * 256 + 'D');
  if (enable) {
    TimerManager::instance.StartTimer(timerId, 1000, true, this,
                                      Clock::GetMilliseconds());
  } else {
    TimerManager::instance.StopTimer(timerId, Clock::GetMilliseconds());
  }
}

void StenoStrokeCapture::WpmTimerHandler() {
#if JAVELIN_SPLIT
  for (int displayId = 0; displayId < 2; ++displayId) {
#else
  const int displayId = 0;
  {
#endif
    switch (autoDraw[displayId]) {
    case AutoDraw::NONE:
    case AutoDraw::PAPER_TAPE:
    case AutoDraw::STENO_LAYOUT:
    case AutoDraw::STROKES:
      break;

    case AutoDraw::WPM:
      DrawWpm(displayId);
      break;
    }
  }
}

void StenoStrokeCapture::DrawWpm(int displayId) {
  const int newWpm = WpmTracker::instance.Get5sWpm();
  if (newWpm == lastWpm[displayId]) {
    return;
  }
  lastWpm[displayId] = newWpm;

  Display::Clear(displayId);

  char buffer[16];
  Str::Sprintf(buffer, "%d", newWpm);
#if JAVELIN_DISPLAY_WIDTH >= 130 && JAVELIN_DISPLAY_HEIGHT >= 128
  const Font *font = &Font::HUGE_DIGITS;
#elif JAVELIN_DISPLAY_WIDTH >= 64
  const Font *font = &Font::LARGE_DIGITS;
#else
  const Font *font = &Font::MEDIUM_DIGITS;
#endif
  DisplayDriver::DrawText(displayId, JAVELIN_DISPLAY_WIDTH / 2,
                          JAVELIN_DISPLAY_HEIGHT / 2 - font->height / 2 +
                              3 * font->baseline / 4,
                          font, TextAlignment::MIDDLE, buffer);

  font = &Font::DEFAULT;
  DisplayDriver::DrawText(displayId, JAVELIN_DISPLAY_WIDTH / 2,
                          JAVELIN_DISPLAY_HEIGHT * 3 / 4 + font->baseline / 2,
                          font, TextAlignment::MIDDLE, "wpm");
}

void StenoStrokeCapture::DrawStrokes(int displayId) {
  Display::Clear(displayId);

  char buffer[16];
  Str::Sprintf(buffer, "%zu", strokeCount);

#if JAVELIN_DISPLAY_WIDTH >= 130 && JAVELIN_DISPLAY_HEIGHT >= 128
  const Font *font =
      Str::Length(buffer) <= 3 ? &Font::HUGE_DIGITS : &Font::LARGE_DIGITS;
#elif JAVELIN_DISPLAY_WIDTH >= 128
  const Font *font = &Font::LARGE_DIGITS;
#elif JAVELIN_DISPLAY_WIDTH >= 64
  const Font *font =
      Str::Length(buffer) <= 3 ? &Font::LARGE_DIGITS : &Font::MEDIUM_DIGITS;
#else
  const Font *font =
      Str::Length(buffer) <= 2 ? &Font::MEDIUM_DIGITS : &Font::SMALL_DIGITS;
#endif
  DisplayDriver::DrawText(displayId, JAVELIN_DISPLAY_WIDTH / 2,
                          JAVELIN_DISPLAY_HEIGHT / 2 - font->height / 2 +
                              3 * font->baseline / 4,
                          font, TextAlignment::MIDDLE, buffer);

  font = &Font::DEFAULT;
  DisplayDriver::DrawText(displayId, JAVELIN_DISPLAY_WIDTH / 2,
                          JAVELIN_DISPLAY_HEIGHT * 3 / 4 + font->baseline / 2,
                          font, TextAlignment::MIDDLE, "strokes");
}

void StenoStrokeCapture::SetAutoDraw_Binding(void *context,
                                             const char *commandLine) {
  StenoStrokeCapture *capture = (StenoStrokeCapture *)context;

  const char *p = strchr(commandLine, ' ');
  if (!p) {
    Console::Printf("ERR No parameters specified\n\n");
    return;
  }
  int displayId = 0;
  ++p;
#if !JAVELIN_SPLIT
  if ('a' <= *p && *p <= 'z')
    goto skipDisplayId;
#endif
  p = Str::ParseInteger(&displayId, p, false);
  if (!p || *p != ' ') {
    Console::Printf("ERR displayId parameter missing\n\n");
    return;
  }
  ++p;

skipDisplayId:
  if (Str::Eq(p, "none")) {
    capture->SetAutoDraw(displayId, AutoDraw::NONE);
  } else if (Str::Eq(p, "paper_tape")) {
    capture->SetAutoDraw(displayId, AutoDraw::PAPER_TAPE);
  } else if (Str::Eq(p, "steno_layout")) {
    capture->SetAutoDraw(displayId, AutoDraw::STENO_LAYOUT);
  } else if (Str::Eq(p, "wpm")) {
    capture->SetAutoDraw(displayId, AutoDraw::WPM);
  } else if (Str::Eq(p, "strokes")) {
    capture->SetAutoDraw(displayId, AutoDraw::STROKES);
  } else {
    Console::Printf("ERR Unable to set auto draw: \"%s\"\n\n", p);
    return;
  }
  Console::SendOk();
}

#if JAVELIN_USE_EMBEDDED_STENO
void Display::SetAutoDraw(int displayId, int autoDrawId) {
  StenoStrokeCapture::container->SetAutoDraw(displayId, (AutoDraw)autoDrawId);
}
#endif

#endif

//---------------------------------------------------------------------------
