//---------------------------------------------------------------------------

#include "ssd1306.h"
#include "javelin/button_script_manager.h"
#include "javelin/clock.h"
#include "javelin/console.h"
#include "javelin/font/monochrome/font.h"
#include "javelin/hal/display.h"
#include "javelin/utf8_pointer.h"
#include "pico_dma.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <string.h>

//---------------------------------------------------------------------------
#if JAVELIN_DISPLAY_DRIVER == 1306

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
Ssd1306::Ssd1306Data Ssd1306::instances[2];
#else
Ssd1306::Ssd1306Data Ssd1306::instances[1];
#endif

uint16_t Ssd1306::dmaBuffer[JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 8 + 1];

//---------------------------------------------------------------------------

const uint32_t I2C_CLOCK = 400'000;

//---------------------------------------------------------------------------

struct Ssd1306Command {
  enum Value : uint8_t {
    SET_LOWER_COLUMN_START_ADDRESS = 0x00,  // -> 0x0f
    SET_HIGHER_COLUMN_START_ADDRESS = 0x10, // -> 0x1f
    SET_MEMORY_ADDRESSING_MODE = 0x20,
    SET_COLUMN_ADDRESS = 0x21,
    SET_PAGE_ADDRESS = 0x22,
    SET_DISPLAY_START_LINE = 0x40, // -> 0x7f
    SET_CONTRAST = 0x81,
    SET_CHARGE_PUMP = 0x8d,
    SET_SEGMENT_REMAP_DISABLED = 0xa0,
    SET_SEGMENT_REMAP_ENABLED = 0xa1,

    SET_FORCE_WHITE_DISABLED = 0xa4,
    SET_FORCE_WHITE_ENABLED = 0xa5,

    SET_INVERT_DISABLED = 0xa6,
    SET_INVERT_ENABLE = 0xa7,

    SET_MULTIPLEX_RATIO = 0xa8,

    SET_DISPLAY_OFF = 0xae,
    SET_DISPLAY_ON = 0xaf,
    SET_PAGE_START_ADDRESS = 0xb0, // -> 0xbf

    SET_COM_SCAN_DIRECTION_TOP_DOWN = 0xc0,
    SET_COM_SCAN_DIRECTION_BOTTOM_UP = 0xc8,

    SET_DISPLAY_OFFSET = 0xd3, // Data: offset

    SET_DISPLAY_OSCILLATOR_FREQUENCY_CLOCK_DIVIDE_RATIO = 0xd5,
    SET_PRECHARGE_PERIOD = 0xd9,
    SET_COM_PIN_CONFIGURATION = 0xda,
    SET_VCOMH_DESELECT_LEVEL = 0xdb,
  };

  constexpr static uint8_t EnableDisplay(bool on) {
    return on ? SET_DISPLAY_ON : SET_DISPLAY_OFF;
  }

  constexpr static uint8_t SetDisplayStartLine(uint8_t line) {
    return SET_DISPLAY_START_LINE + line;
  }
};

struct Ssd1306MemoryAddressingMode {
  enum Value : uint8_t {
    HORIZONTAL = 0,
    VERTICAL = 1,
    PAGE = 2,
  };
};

//---------------------------------------------------------------------------

bool Ssd1306::SendCommand(uint8_t command) {
  uint8_t buffer[2] = {0x80, command};
  return i2c_write_blocking(JAVELIN_OLED_I2C, JAVELIN_OLED_I2C_ADDRESS, buffer,
                            2, false) == 2;
}

bool Ssd1306::SendCommandList(const uint8_t *commands, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (!SendCommand(commands[i])) {
      return false;
    }
  }
  return true;
}

void Ssd1306::SendCommandListDma(const uint8_t *commands, size_t length) {
  JAVELIN_OLED_I2C->hw->enable = 0;
  JAVELIN_OLED_I2C->hw->tar = JAVELIN_OLED_I2C_ADDRESS;
  JAVELIN_OLED_I2C->hw->enable = 1;

  // Start of data.
  uint16_t *d = dmaBuffer;
  for (size_t i = 0; i < length; ++i) {
    *d++ = 0x80;
    *d++ = commands[i] | 0x200;
  }

  SendDmaBuffer(length * 2);
}

bool Ssd1306::IsI2cTxReady() {
  return (JAVELIN_OLED_I2C->hw->raw_intr_stat &
          I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS) != 0;
}

void Ssd1306::WaitForI2cTxReady() {
  while (!IsI2cTxReady()) {
    // Do nothing.
  }
}

void Ssd1306::SendDmaBuffer(size_t count) {
  dma4->count = count;
  dma4->sourceTrigger = dmaBuffer;
}

void Ssd1306::PrintInfo() {
#if JAVELIN_SPLIT
  Console::Printf("Screen: %s, %s\n",
                  instances[0].available ? "present" : "not present",
                  instances[1].available ? "present" : "not present");
#else
  Console::Printf("Screen: %s\n",
                  instances[0].available ? "present" : "not present");
#endif
}

//---------------------------------------------------------------------------

void Ssd1306::Ssd1306Data::SetPixel(uint32_t x, uint32_t y) {
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const size_t bitIndex = x * JAVELIN_DISPLAY_HEIGHT + y;
  uint8_t *p = &buffer8[bitIndex / 8];
  if (drawColor) {
    *p |= 1 << (bitIndex & 7);
  } else {
    *p &= ~(1 << (bitIndex & 7));
  }
}

void Ssd1306::Ssd1306Data::Clear() {
  if (!available) {
    return;
  }
  dirty = true;
  Mem::Clear(buffer32);
}

void Ssd1306::Ssd1306Data::DrawLine(int x0, int y0, int x1, int y1) {
  if (!available) {
    return;
  }
  dirty = true;

  // Use balanced Bresenham's line algorithm:
  // https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
  const int dx = abs(x1 - x0);
  const int sx = x0 < x1 ? 1 : -1;
  const int dy = -abs(y1 - y0);
  const int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;

  while (true) {
    SetPixel(x0, y0);
    if (x0 == x1 && y0 == y1) {
      return;
    }

    const int e2 = 2 * error;
    if (e2 >= dy) {
      error += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

void Ssd1306::Ssd1306Data::DrawRect(int left, int top, int right, int bottom) {
  if (!available) {
    return;
  }

  if (right < 0 || bottom < 0 || left >= JAVELIN_DISPLAY_WIDTH ||
      top >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }
  if (left < 0) {
    left = 0;
  }
  if (right >= JAVELIN_DISPLAY_WIDTH) {
    right = JAVELIN_DISPLAY_WIDTH;
  }
  int width = right - left;

  if (top < 0) {
    top = 0;
  }
  if (bottom >= JAVELIN_DISPLAY_HEIGHT) {
    bottom = JAVELIN_DISPLAY_HEIGHT;
  }

  dirty = true;

  uint8_t *p = &buffer8[left * (JAVELIN_DISPLAY_HEIGHT / 8) + (top >> 3)];

  const int startY = top & -8;
  const int endY = (bottom + 7) & -8;

  for (int y = startY; y < endY; y += 8) {
    int mask = 0xff;
    if (y < top) {
      mask &= mask << (top - y);
    }
    if (y + 8 > bottom) {
      mask &= mask >> (y + 8 - bottom);
    }
    uint8_t *row = p;
    for (int x = 0; x < width; ++x) {
      int pixels = *row;
      if (drawColor) {
        pixels |= mask;
      } else {
        pixels &= ~mask;
      }
      *row = pixels;
      row += JAVELIN_DISPLAY_HEIGHT / 8;
    }
    p++;
  }
}

void Ssd1306::Ssd1306Data::DrawBitmapImage(int x, int y, int width, int height,
                                           const uint8_t *data) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  if (y + height <= 0) {
    return;
  }

  size_t bytesPerColumn = (height + 7) >> 3;

  if (x < 0) {
    width += x;
    if (width <= 0) {
      return;
    }
    data -= bytesPerColumn * x;
    x = 0;
  }

  dirty = true;

  const int startY = y >> 3;
  const int yShift = y & 7;
  const int endY = (y + height + 7) >> 3;

  uint8_t *p = &buffer8[x * (JAVELIN_DISPLAY_HEIGHT / 8)];

  const int endX = x + width;
  if (endX > JAVELIN_DISPLAY_WIDTH) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }

  while (width > 0) {
    const uint8_t *column = data;
    data += bytesPerColumn;

    if (startY >= 0) {
      const int bits = column[0] << yShift;
      int pixels = p[startY];
      if (drawColor) {
        pixels |= bits;
      } else {
        pixels &= ~bits;
      }
      p[startY] = pixels;
    }

    for (int yy = startY + 1; yy < endY; ++yy) {
      ++column;
      if (yy >= JAVELIN_DISPLAY_HEIGHT / 8) {
        break;
      }
      if (yy < 0) {
        continue;
      }

      int bits = (column[-1] >> (8 - yShift));
      if (column < data) {
        bits |= column[0] << yShift;
      }
      int pixels = p[yy];
      if (drawColor) {
        pixels |= bits;
      } else {
        pixels &= ~bits;
      }
      p[yy] = pixels;
    }

    p += JAVELIN_DISPLAY_HEIGHT / 8;
    --width;
  }
}

void Ssd1306::Ssd1306Data::DrawLuminanceRange(int x, int y, int width,
                                              int height, const uint8_t *data,
                                              int min, int max) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const int imageWidth = width;
  if (y < 0) {
    height += y;
    if (height <= 0) {
      return;
    }
    data -= y * width;
    y = 0;
  }
  if (height > JAVELIN_DISPLAY_HEIGHT - y) {
    height = JAVELIN_DISPLAY_HEIGHT - y;
  }

  if (x < 0) {
    width += x;
    if (width <= 0) {
      return;
    }
    data -= x;
    x = 0;
  }
  if (width > JAVELIN_DISPLAY_WIDTH - x) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }
  const int dataAdvance = imageWidth - width;

  dirty = true;
  uint8_t *p = &buffer8[x * (JAVELIN_DISPLAY_HEIGHT / 8)];

  for (int yy = 0; yy < height; ++yy, data += dataAdvance) {
    const uint32_t yyy = y + yy;
    uint8_t *px = p + (yyy >> 3);
    const int bit = 1 << (yyy & 7);
    for (int xx = 0; xx < width; ++xx, px += (JAVELIN_DISPLAY_HEIGHT / 8)) {
      const uint32_t v = *data++;
      if (min <= v && v < max) {
        if (drawColor) {
          *px |= bit;
        } else {
          *px &= ~bit;
        }
      }
    }
  }
}

void Ssd1306::Ssd1306Data::DrawText(int x, int y, const Font *font,
                                    TextAlignment alignment, const char *text) {
  if (!available) {
    return;
  }

  Utf8Pointer utf8p(text);
  y -= font->baseline;

  switch (alignment) {
  case TextAlignment::LEFT:
    break;
  case TextAlignment::MIDDLE:
    x -= font->GetStringWidth(text) >> 1;
    break;
  case TextAlignment::RIGHT:
    x -= font->GetStringWidth(text);
    break;
  }

  for (;;) {
    const uint32_t c = *utf8p++;
    if (c == 0) {
      return;
    }

    const uint8_t *data = font->GetCharacterData(c);
    if (data) {
      uint32_t width = font->GetCharacterWidth(c);
      DrawBitmapImage(x, y, width, font->height, data);
      x += width;
    }
    x += font->spacing;
  }
}

void Ssd1306::Ssd1306Data::Update() {
  if (!available) {
    return;
  }

  control.Update();

  if (!dirty || dma4->IsBusy() || !IsI2cTxReady()) {
    return;
  }

  ButtonScriptManager::GetInstance().ExecuteScript(
      ButtonScriptId::DISPLAY_OVERLAY);

  dirty = false;

  JAVELIN_OLED_I2C->hw->enable = 0;
  JAVELIN_OLED_I2C->hw->tar = JAVELIN_OLED_I2C_ADDRESS;
  JAVELIN_OLED_I2C->hw->enable = 1;

  // Start of data.
  uint16_t *d = dmaBuffer;
  *d++ = 0x40;

#if JAVELIN_OLED_ROTATION == 0 || JAVELIN_OLED_ROTATION == 180
  const uint8_t *s = buffer8;
  // Copy the frame buffer to DMA to the display.
  for (size_t i = 0; i < JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 8; ++i) {
    *d++ = *s++;
  }
#elif JAVELIN_OLED_ROTATION == 90 || JAVELIN_OLED_ROTATION == 270
  for (int frameBufferY = JAVELIN_DISPLAY_HEIGHT - 1; frameBufferY >= 0;
       --frameBufferY) {
    const int shift = ~frameBufferY & 7;

    const uint8_t *s = &buffer8[frameBufferY / 8];

    for (size_t y = 0; y < JAVELIN_OLED_HEIGHT / 8; ++y) {
      uint32_t pixelValue = 0;
      for (size_t i = 0; i < 8; ++i) {
        pixelValue = (pixelValue >> 1) | ((*s << shift) & 0x80);
        s += JAVELIN_DISPLAY_HEIGHT / 8;
      }
      *d++ = pixelValue;
    }
  }
#else
#error Unhandled rotation
#endif

  // Mark last byte as end of data.
  d[-1] |= 0x200;

  SendDmaBuffer(JAVELIN_OLED_WIDTH * JAVELIN_OLED_HEIGHT / 8 + 1);
}

bool Ssd1306::Ssd1306Data::InitializeSsd1306() {
  static constexpr uint8_t COMMANDS[] = {
      Ssd1306Command::EnableDisplay(false),

      Ssd1306Command::SET_MEMORY_ADDRESSING_MODE,
      Ssd1306MemoryAddressingMode::VERTICAL,

      Ssd1306Command::SetDisplayStartLine(0),
#if !defined(JAVELIN_OLED_ROTATION) || JAVELIN_OLED_ROTATION == 0 ||           \
    JAVELIN_OLED_ROTATION == 90
      Ssd1306Command::SET_SEGMENT_REMAP_ENABLED,
      Ssd1306Command::SET_COM_SCAN_DIRECTION_BOTTOM_UP,
#elif JAVELIN_OLED_ROTATION == 180 || JAVELIN_OLED_ROTATION == 270
      Ssd1306Command::SET_SEGMENT_REMAP_DISABLED,
      Ssd1306Command::SET_COM_SCAN_DIRECTION_TOP_DOWN,
#else
#error Unsupported OLED rotation
#endif
      Ssd1306Command::SET_MULTIPLEX_RATIO,
      JAVELIN_OLED_HEIGHT - 1, // Display height - 1
      Ssd1306Command::SET_DISPLAY_OFFSET,
      0x00,

      Ssd1306Command::SET_COM_PIN_CONFIGURATION,
#if (JAVELIN_OLED_WIDTH == 128 && JAVELIN_OLED_HEIGHT == 32)
      0x02,
#elif (JAVELIN_OLED_WIDTH == 128 && JAVELIN_OLED_HEIGHT == 64)
      0x12,
#else
      0x02,
#endif

      Ssd1306Command::SET_DISPLAY_OSCILLATOR_FREQUENCY_CLOCK_DIVIDE_RATIO,
      0x80, // Standard frequency, ClockDiv = 1.
      Ssd1306Command::SET_PRECHARGE_PERIOD,
      0xF1,
      Ssd1306Command::SET_VCOMH_DESELECT_LEVEL,
      0x30, // 0.83xVcc

      Ssd1306Command::SET_CONTRAST,
      0xFF,

      Ssd1306Command::SET_FORCE_WHITE_DISABLED,
      Ssd1306Command::SET_INVERT_DISABLED,
      Ssd1306Command::SET_CHARGE_PUMP,
      0x14, // Enable charge pump during display on.

      Ssd1306Command::EnableDisplay(true),

      Ssd1306Command::SET_COLUMN_ADDRESS,
      0,
      JAVELIN_OLED_WIDTH - 1,
      Ssd1306Command::SET_PAGE_ADDRESS,
      0,
      (JAVELIN_OLED_HEIGHT - 1) / 8,
  };

  return SendCommandList(COMMANDS, sizeof(COMMANDS));
}

void Ssd1306::Ssd1306Data::Initialize() {
  i2c_init(JAVELIN_OLED_I2C, I2C_CLOCK);
  gpio_set_function(JAVELIN_OLED_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(JAVELIN_OLED_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(JAVELIN_OLED_SDA_PIN);
  gpio_pull_up(JAVELIN_OLED_SCL_PIN);

  dma4->destination = &JAVELIN_OLED_I2C->hw->data_cmd;

  constexpr PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::HALF_WORD,
      .incrementRead = true,
      .incrementWrite = false,
      .chainToDma = 4,
      .transferRequest = PicoDmaTransferRequest::JAVELIN_OLED_TX_DMA,
      .sniffEnable = false,
  };
  dma4->control = dmaControl;

  available = InitializeSsd1306();
  if (!available) {
    return;
  }

  // Update a black screen to avoid initial noise on the display.
  WaitForI2cTxReady();
  dirty = true;
  Update();
}

//---------------------------------------------------------------------------

void Ssd1306::Ssd1306Control::Update() {
  if (dirtyFlag == 0 || dma4->IsBusy() || !IsI2cTxReady()) {
    return;
  }

  uint8_t commands[4];
  uint8_t *p = commands;
  if (dirtyFlag & DIRTY_FLAG_SCREEN_ON) {
    *p++ = Ssd1306Command::EnableDisplay(data.screenOn);
  }
  if (dirtyFlag & DIRTY_FLAG_CONTRAST) {
    *p++ = Ssd1306Command::SET_CONTRAST;
    *p++ = data.contrast;
  }
  dirtyFlag = 0;

  SendCommandListDma(commands, p - commands);
}

//---------------------------------------------------------------------------

void Ssd1306::Ssd1306Availability::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;
  buffer.Add(SplitHandlerId::DISPLAY_AVAILABLE, &available, sizeof(bool));
}

void Ssd1306::Ssd1306Availability::OnDataReceived(const void *data,
                                                  size_t length) {
  available = *(bool *)data;
}

void Ssd1306::Ssd1306Control::UpdateBuffer(TxBuffer &buffer) {
  if (dirtyFlag == 0) {
    return;
  }
  dirtyFlag = 0;
  buffer.Add(SplitHandlerId::DISPLAY_CONTROL, &data, sizeof(data));
}

void Ssd1306::Ssd1306Control::OnDataReceived(const void *data, size_t length) {
  const Ssd1306ControlTxRxData *controlData =
      (const Ssd1306ControlTxRxData *)data;

  if (this->data.screenOn != controlData->screenOn) {
    this->data.screenOn = controlData->screenOn;
    dirtyFlag |= DIRTY_FLAG_SCREEN_ON;
  }

  if (this->data.contrast != controlData->contrast) {
    this->data.contrast = controlData->contrast;
    dirtyFlag |= DIRTY_FLAG_CONTRAST;
  }
}

void Ssd1306::Ssd1306Control::OnTransmitConnectionReset() {
  dirtyFlag = DIRTY_FLAG_SCREEN_ON | DIRTY_FLAG_CONTRAST;
}

void Ssd1306::Ssd1306Data::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty || !available) {
    return;
  }
  dirty = false;
  buffer.Add(SplitHandlerId::DISPLAY_DATA, buffer8, sizeof(buffer8));
}

void Ssd1306::Ssd1306Data::OnDataReceived(const void *data, size_t length) {
  dirty = true;
  memcpy(buffer8, data, sizeof(buffer8));
}

//---------------------------------------------------------------------------

void Display::Clear(int displayId) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  Ssd1306::instances[displayId].Clear();
}

void Display::SetScreenOn(int displayId, bool on) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  Ssd1306::instances[displayId].control.SetScreenOn(on);
}

void Display::SetContrast(int displayId, int contrast) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  Ssd1306::instances[displayId].control.SetContrast(contrast > 255 ? 255
                                                                   : contrast);
}

void Display::DrawPixel(int displayId, int x, int y) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  Ssd1306::instances[displayId].dirty = true;
  Ssd1306::instances[displayId].SetPixel(x, y);
}

void Display::DrawLine(int displayId, int x1, int y1, int x2, int y2) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  Ssd1306::instances[displayId].DrawLine(x1, y1, x2, y2);
}

void Display::DrawImage(int displayId, int x, int y, int width, int height,
                        ImageFormat format, const uint8_t *data) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  switch (format) {
  case ImageFormat::BITMAP:
    Ssd1306::instances[displayId].DrawBitmapImage(x, y, width, height, data);
    break;
  case ImageFormat::LUMINANCE8:
  case ImageFormat::RGB332:
  case ImageFormat::RGB565:
  case ImageFormat::RGB888:
  case ImageFormat::ALPHA8:
  case ImageFormat::ARGB1555:
  case ImageFormat::RGBA8888:
    break;
  }
}

void Display::DrawLuminanceRange(int displayId, int x, int y, int width,
                                 int height, const uint8_t *data, int min,
                                 int max) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  Ssd1306::instances[displayId].DrawLuminanceRange(x, y, width, height, data,
                                                   min, max);
}

void Display::DrawText(int displayId, int x, int y, FontId fontId,
                       TextAlignment alignment, const char *text) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  const Font *font = Font::GetFont(fontId);
  Ssd1306::instances[displayId].DrawText(x, y, font, alignment, text);
}

void Display::DrawRect(int displayId, int left, int top, int right,
                       int bottom) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  Ssd1306::instances[displayId].DrawRect(left, top, right, bottom);
}

void Display::SetDrawColor(int displayId, int color) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  Ssd1306::instances[displayId].drawColor = color != 0;
}

//---------------------------------------------------------------------------

#endif // JAVELIN_DISPLAY_DRIVER == 1306

//---------------------------------------------------------------------------
