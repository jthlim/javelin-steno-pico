//---------------------------------------------------------------------------

#include "st7789.h"
#include "javelin/button_script_manager.h"
#include "javelin/console.h"
#include "javelin/font/monochrome/font.h"
#include "javelin/hal/display.h"
#include "javelin/malloc_allocate.h"
#include "javelin/thread.h"
#include "javelin/utf8_pointer.h"
#include "pico_dma.h"
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <pico/time.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_DISPLAY_DRIVER == 7789

//---------------------------------------------------------------------------

enum class St7789Command : uint8_t {
  SOFTWARE_RESET = 0x01,
  SLEEP_ON = 0x10,
  SLEEP_OFF = 0x11,
  SET_DISPLAY_MODE_PARTIAL = 0x12,
  SET_DISPLAY_MODE_NORMAL = 0x13,
  DISPLAY_INVERSION_OFF = 0x20,
  DISPLAY_INVERSION_ON = 0x21,
  DISPLAY_OFF = 0x28,
  DISPLAY_ON = 0x29,
  SET_COLUMN_ADDRESS = 0x2a,
  SET_ROW_ADDRESS = 0x2b,
  MEMORY_WRITE = 0x2c,
  MEMORY_DATA_ACCESS_CONTROL = 0x36,
  SET_INTERFACE_PIXEL_FORMAT = 0x3a,
  SET_DISPLAY_BRIGHTNESS = 0x51,
};

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT
St7789::St7789Data St7789::instances[2];
#else
St7789::St7789Data St7789::instances[1];
#endif

//---------------------------------------------------------------------------

void St7789::SendCommand(St7789Command command, const void *data,
                         size_t length) {
  gpio_put(JAVELIN_DISPLAY_DC_PIN, 0);
  spi_set_format(JAVELIN_DISPLAY_SPI, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

  spi_write_blocking(JAVELIN_DISPLAY_SPI, (const uint8_t *)&command,
                     sizeof(command));

  if (length) {
    gpio_put(JAVELIN_DISPLAY_DC_PIN, 1);
    spi_write_blocking(JAVELIN_DISPLAY_SPI, (const uint8_t *)data, length);
  }
}

void St7789::SetLR(uint32_t left, uint32_t right) {
  const uint8_t data[] = {
      (uint8_t)(left >> 8),
      (uint8_t)(left & 0xff),
      (uint8_t)(right >> 8),
      (uint8_t)(right & 0xff),
  };

  SendCommand(St7789Command::SET_COLUMN_ADDRESS, data, sizeof(data));
}

void St7789::SetTB(uint32_t top, uint32_t bottom) {
  const uint8_t data[] = {
      (uint8_t)(top >> 8),
      (uint8_t)(top & 0xff),
      (uint8_t)(bottom >> 8),
      (uint8_t)(bottom & 0xff),
  };

  SendCommand(St7789Command::SET_ROW_ADDRESS, data, sizeof(data));
}

static bool IsSpiInitialized(spi_inst_t *instance) {
  return (spi_get_hw(instance)->cr1 & SPI_SSPCR1_SSE_BITS) != 0;
}

void St7789::St7789Data::Initialize() {
  if (IsSpiInitialized(JAVELIN_DISPLAY_SPI)) {
    available = false;
    return;
  }

#if defined(JAVELIN_DISPLAY_DETECT)
  if (!JAVELIN_DISPLAY_DETECT) {
    available = false;
    return;
  }
#endif

  const uint32_t systemClockKhz =
      frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
  spi_init(JAVELIN_DISPLAY_SPI, 1000 * systemClockKhz);

  gpio_init(JAVELIN_DISPLAY_MISO_PIN);
  gpio_set_function(JAVELIN_DISPLAY_MISO_PIN, GPIO_FUNC_SPI);

  gpio_init(JAVELIN_DISPLAY_MOSI_PIN);
  gpio_set_function(JAVELIN_DISPLAY_MOSI_PIN, GPIO_FUNC_SPI);

  gpio_init(JAVELIN_DISPLAY_SCK_PIN);
  gpio_set_function(JAVELIN_DISPLAY_SCK_PIN, GPIO_FUNC_SPI);

  gpio_init(JAVELIN_DISPLAY_DC_PIN);
  gpio_set_dir(JAVELIN_DISPLAY_DC_PIN, GPIO_OUT);
  gpio_put(JAVELIN_DISPLAY_DC_PIN, 0); // Command = 0, Data = 1

  gpio_init(JAVELIN_DISPLAY_CS_PIN);
  gpio_set_dir(JAVELIN_DISPLAY_CS_PIN, GPIO_OUT);
  gpio_put(JAVELIN_DISPLAY_CS_PIN, 0);

  gpio_init(JAVELIN_DISPLAY_RST_PIN);
  gpio_set_dir(JAVELIN_DISPLAY_RST_PIN, GPIO_OUT);
  gpio_put(JAVELIN_DISPLAY_RST_PIN, 0);
  sleep_us(100);
  gpio_put(JAVELIN_DISPLAY_RST_PIN, 1);
  sleep_ms(120);

  SendCommand(St7789Command::SOFTWARE_RESET);
  sleep_ms(5);

  SendCommand(St7789Command::SLEEP_OFF, NULL, 0);
  sleep_ms(5);

  static const uint8_t interfaceFormat = 0x55; // 565 rgb interface + control.
  SendCommand(St7789Command::SET_INTERFACE_PIXEL_FORMAT, &interfaceFormat, 1);

#if JAVELIN_DISPLAY_ROTATION == 0
  constexpr int xOffset = (240 - JAVELIN_DISPLAY_SCREEN_WIDTH) / 2;
  constexpr int yOffset = (320 - JAVELIN_DISPLAY_SCREEN_HEIGHT) / 2;
#elif JAVELIN_DISPLAY_ROTATION == 90
  constexpr int xOffset = (320 - JAVELIN_DISPLAY_WIDTH) / 2;
  constexpr int yOffset = (240 - JAVELIN_DISPLAY_HEIGHT) / 2;
  static const uint8_t memoryDataAccess =
      0xa0; // Bottom to top, left to right, Reverse.
  SendCommand(St7789Command::MEMORY_DATA_ACCESS_CONTROL, &memoryDataAccess, 1);
#elif JAVELIN_DISPLAY_ROTATION == 180
  constexpr int xOffset = (240 - JAVELIN_DISPLAY_SCREEN_WIDTH) / 2;
  constexpr int yOffset = (320 - JAVELIN_DISPLAY_SCREEN_HEIGHT) / 2;
  static const uint8_t memoryDataAccess =
      0xc0; // Bottom to top, right to left, normal.
  SendCommand(St7789Command::MEMORY_DATA_ACCESS_CONTROL, &memoryDataAccess, 1);
#elif JAVELIN_DISPLAY_ROTATION == 270
  constexpr int xOffset = (320 - JAVELIN_DISPLAY_WIDTH) / 2;
  constexpr int yOffset = (240 - JAVELIN_DISPLAY_HEIGHT) / 2;
  static const uint8_t memoryDataAccess =
      0x60; // Top to bottom, right to left, Reverse.
  SendCommand(St7789Command::MEMORY_DATA_ACCESS_CONTROL, &memoryDataAccess, 1);
#else
#error Rotation not supported
#endif

  SendCommand(St7789Command::DISPLAY_INVERSION_ON);
  SendCommand(St7789Command::SET_DISPLAY_MODE_NORMAL, NULL, 0);

  SetLR(xOffset, xOffset + JAVELIN_DISPLAY_WIDTH - 1);
  SetTB(yOffset, yOffset + JAVELIN_DISPLAY_HEIGHT - 1);

  dma4->destination = &spi_get_hw(JAVELIN_DISPLAY_SPI)->dr;
  dma4->count = sizeof(buffer32) / 2;
  constexpr PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::HALF_WORD,
      .incrementRead = true,
      .incrementWrite = false,
      .chainToDma = 4,
      .transferRequest = PicoDmaTransferRequest::JAVELIN_DISPLAY_TX_DMA,
      .sniffEnable = false,
  };
  dma4->control = dmaControl;

  // Clear buffer on startup.
  SendScreenData();
  dma4->WaitUntilComplete();
  // After dma is complete, still need the last 16 bits to send.
  sleep_us(10);

  SendCommand(St7789Command::DISPLAY_ON, NULL, 0);

#if defined(JAVELIN_DISPLAY_BACKLIGHT_PIN)
  gpio_init(JAVELIN_DISPLAY_BACKLIGHT_PIN);
  gpio_set_dir(JAVELIN_DISPLAY_BACKLIGHT_PIN, GPIO_OUT);
  gpio_put(JAVELIN_DISPLAY_BACKLIGHT_PIN, 1);
#endif

  available = true;
}

void St7789::PrintInfo() {
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

void St7789::St7789Data::SetPixel(uint32_t x, uint32_t y) {
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const size_t pixelIndex = y * JAVELIN_DISPLAY_WIDTH + x;
  buffer16[pixelIndex] = drawColor565;
}

void St7789::St7789Data::Clear() {
  static int zero = 0;

  if (!available) {
    return;
  }
  dirty = true;

  dma5->source = &zero;
  dma5->destination = buffer32;
  dma5->count = sizeof(buffer32) / 4;
  constexpr PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::WORD,
      .incrementRead = false,
      .incrementWrite = true,
      .chainToDma = 5,
      .transferRequest = PicoDmaTransferRequest::PERMANENT,
      .sniffEnable = false,
  };
  dma5->controlTrigger = dmaControl;
  dma5->WaitUntilComplete();
}

void St7789::St7789Data::DrawLine(int x0, int y0, int x1, int y1) {
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

void St7789::St7789Data::DrawRect(int left, int top, int right, int bottom) {
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
  const int width = right - left;

  if (top < 0) {
    top = 0;
  }
  if (bottom >= JAVELIN_DISPLAY_HEIGHT) {
    bottom = JAVELIN_DISPLAY_HEIGHT;
  }

  dirty = true;

  uint16_t *p = &buffer16[top * JAVELIN_DISPLAY_WIDTH + left];
  for (int y = top; y < bottom; y++) {
    for (int x = 0; x < width; ++x) {
      p[x] = drawColor565;
    }
    p += JAVELIN_DISPLAY_WIDTH;
  }
}

void St7789::St7789Data::DrawBitmapImage(int x, int y, int width, int height,
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

  const size_t bytesPerColumn = (height + 7) >> 3;

  if (x < 0) {
    width += x;
    if (width <= 0) {
      return;
    }
    data -= bytesPerColumn * x;
    x = 0;
  }

  dirty = true;

  uint16_t *p = &buffer16[x];

  const int endX = x + width;
  if (endX > JAVELIN_DISPLAY_WIDTH) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }

  while (width > 0) {
    for (int yy = 0, topY = y; yy < bytesPerColumn; ++yy, topY += 8) {
      int v = *data++;
#pragma GCC unroll 8
      for (int i = 0; i < 8; ++i) {
        if (v & (1 << i)) {
          const uint32_t pixelY = topY + i;
          if (pixelY < JAVELIN_DISPLAY_HEIGHT) {
            p[pixelY * JAVELIN_DISPLAY_WIDTH] = drawColor565;
          }
        }
      }
    }

    p++;
    --width;
  }
}

void St7789::St7789Data::DrawLuminanceImage(int x, int y, int width, int height,
                                            const uint8_t *data) {
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

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, data += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      *p++ = drawColor.Scale(*data++).To565();
    }
  }
}

void St7789::St7789Data::DrawRgb332Image(int x, int y, int width, int height,
                                         const uint8_t *data) {
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

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, data += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      const uint32_t v = *data++;
      const uint32_t r = (v >> 5) & 7;
      const uint32_t g = (v >> 2) & 7;
      const uint32_t b = v & 3;
      *p++ = (r << 13) | (g << 8) | (b << 3);
    }
  }
}

void St7789::St7789Data::DrawRgb565Image(int x, int y, int width, int height,
                                         const uint8_t *data) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const int imageWidth = width;
  const uint16_t *rgbData = (const uint16_t *)data;
  if (y < 0) {
    height += y;
    if (height <= 0) {
      return;
    }
    rgbData -= y * width;
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
    rgbData -= x;
    x = 0;
  }
  if (width > JAVELIN_DISPLAY_WIDTH - x) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }

  dirty = true;

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];

  dma5->count = width;
  constexpr PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::HALF_WORD,
      .incrementRead = true,
      .incrementWrite = true,
      .chainToDma = 5,
      .transferRequest = PicoDmaTransferRequest::PERMANENT,
      .sniffEnable = false,
  };
  dma5->control = dmaControl;

  while (height > 0) {
    dma5->source = rgbData;
    dma5->destinationTrigger = p;
    rgbData += imageWidth;
    p += JAVELIN_DISPLAY_WIDTH;
    dma5->WaitUntilComplete();
    --height;
  }
}

void St7789::St7789Data::DrawRgb888Image(int x, int y, int width, int height,
                                         const uint8_t *data) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const int imageWidth = width;
  const Color *rgbData = (const Color *)data;
  if (y < 0) {
    height += y;
    if (height <= 0) {
      return;
    }
    rgbData -= y * width;
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
    rgbData -= x;
    x = 0;
  }

  if (width > JAVELIN_DISPLAY_WIDTH - x) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }
  const int dataAdvance = imageWidth - width;

  dirty = true;

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, rgbData += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      *p++ = rgbData->To565();
      ++rgbData;
    }
  }
}

void St7789::St7789Data::DrawAlpha8Image(int x, int y, int width, int height,
                                         const uint8_t *data) {
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

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, data += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      const uint32_t a = *data++;
      if (a != 0) {
        if (a == 255) {
          *p = drawColor565;
        } else {
          const Color c = Color::From565(*p);
          *p = c.Blend(drawColor, a).To565();
        }
      }
      ++p;
    }
  }
}

void St7789::St7789Data::DrawArgb1555Image(int x, int y, int width, int height,
                                           const uint8_t *data) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const int imageWidth = width;
  const uint16_t *rgbData = (const uint16_t *)data;
  if (y < 0) {
    height += y;
    if (height <= 0) {
      return;
    }
    rgbData -= y * width;
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
    rgbData -= x;
    x = 0;
  }
  if (width > JAVELIN_DISPLAY_WIDTH - x) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }
  const int dataAdvance = imageWidth - width;

  dirty = true;

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, rgbData += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      const uint16_t v = *rgbData++;
      if (v & 0x8000) {
        // Convert argb1555 to rgb565
        *p = ((v << 1) & 0xffc0) | (v & 0x1f);
      }
      ++p;
    }
  }
}

void St7789::St7789Data::DrawRgba8888Image(int x, int y, int width, int height,
                                           const uint8_t *data) {
  if (!available) {
    return;
  }

  // It's all off the screen.
  if (x >= JAVELIN_DISPLAY_WIDTH || y >= JAVELIN_DISPLAY_HEIGHT) {
    return;
  }

  const int imageWidth = width;
  const ColorRgba *rgbaData = (const ColorRgba *)data;
  if (y < 0) {
    height += y;
    if (height <= 0) {
      return;
    }
    rgbaData -= y * width;
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
    rgbaData -= x;
    x = 0;
  }

  if (width > JAVELIN_DISPLAY_WIDTH - x) {
    width = JAVELIN_DISPLAY_WIDTH - x;
  }
  const int dataAdvance = imageWidth - width;

  dirty = true;

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, rgbaData += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      const Color c = Color::From565(*p);
      *p++ = c.Blend(*rgbaData, rgbaData->a).To565();
      ++rgbaData;
    }
  }
}

void St7789::St7789Data::DrawLuminanceRange(int x, int y, int width, int height,
                                            const uint8_t *data, int min,
                                            int max) {
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

  uint16_t *p = &buffer16[y * JAVELIN_DISPLAY_WIDTH + x];
  const int bufferAdvance = JAVELIN_DISPLAY_WIDTH - width;

  for (; height > 0; --height, data += dataAdvance, p += bufferAdvance) {
    for (int xx = 0; xx < width; ++xx) {
      const uint32_t v = *data++;
      if (min <= v && v < max) {
        *p = drawColor565;
      }
      ++p;
    }
  }
}

void St7789::St7789Data::DrawText(int x, int y, const Font *font,
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
      const uint32_t width = font->GetCharacterWidth(c);
      DrawBitmapImage(x, y, width, font->height, data);
      x += width;
    }
    x += font->spacing;
  }
}

struct St7789::St7789Data::RunConwayStepThreadData
    : public JavelinMallocAllocate {
  struct RgbaThresholds {
    uint32_t value;
    void FromRgb565(uint16_t pixel) { value = pixel & 0x8410; }
  };

  uint16_t *buffer;
  int startYm1;
  int startY;
  int endY;

  uint16_t lastLineBuffer[JAVELIN_DISPLAY_WIDTH];
  RgbaThresholds conwayBuffers[3][JAVELIN_DISPLAY_WIDTH + 2];
  uint16_t outputBuffer0[JAVELIN_DISPLAY_WIDTH];
  uint16_t outputBuffer1[JAVELIN_DISPLAY_WIDTH];

  static void SetBuffer(RgbaThresholds *output, const uint16_t *input,
                        size_t length) {
    output->FromRgb565(input[length - 1]);
    for (size_t i = 0; i < length; ++i) {
      output[i + 1].FromRgb565(input[i]);
    }
    output[length + 1].FromRgb565(input[0]);
  }

  // Dead->alive will spike to 31, then decay to 24
  static constexpr uint8_t ALIVE[32] = {
      31, 31, 31, 31, 31, 31, 31, 31, //
      31, 31, 31, 31, 31, 31, 31, 31, //
      16, 17, 18, 19, 20, 21, 22, 23, //
      24, 24, 25, 26, 27, 28, 29, 30, //
  };

  // Alive->Dead will drop to 15, then decay to 0.
  static constexpr uint8_t DEAD[32] = {
      0,  0,  1,  2,  3,  4,  5,  6,  //
      7,  8,  9,  10, 11, 12, 13, 14, //
      15, 15, 15, 15, 15, 15, 15, 15, //
      15, 15, 15, 15, 15, 15, 15, 15, //
  };

  // This is a simplified table that ORs in the current pixel alive to the
  // number of alive neighbors to simplify the lookup.
  static constexpr const uint8_t *(NEXT_EVOLUTION[10]) = {
      // previously dead, previously alive
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, ALIVE},
      ALIVE, // {ALIVE, ALIVE},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD},
      DEAD,  // {DEAD, DEAD}, // Needed since the OR can generate this index.
  };

  static void ThreadEntryPoint(void *data) {
    ((RunConwayStepThreadData *)data)->Run();
  }

  void Run() {
    uint16_t *rowOutput = outputBuffer0;

    SetBuffer(conwayBuffers[0], buffer + startYm1 * JAVELIN_DISPLAY_WIDTH,
              JAVELIN_DISPLAY_WIDTH);
    SetBuffer(conwayBuffers[1], buffer + startY * JAVELIN_DISPLAY_WIDTH,
              JAVELIN_DISPLAY_WIDTH);

    RgbaThresholds *lm1 = conwayBuffers[0];
    RgbaThresholds *l = conwayBuffers[1];
    RgbaThresholds *lp1 = conwayBuffers[2];

    asm volatile(".align 2");

    for (size_t y = startY; y < endY; ++y) {
      const uint16_t *row = buffer + y * JAVELIN_DISPLAY_WIDTH;
      const uint16_t *nextLineRgb565 =
          (y == endY - 1) ? lastLineBuffer
                          : &buffer[(y + 1) * JAVELIN_DISPLAY_WIDTH];
      SetBuffer(lp1, nextLineRgb565, JAVELIN_DISPLAY_WIDTH);

      for (int x = 0; x < JAVELIN_DISPLAY_WIDTH; ++x) {
        // This is brute force - could use area sum to reduce this to 5 terms:
        //   count = BRsum + TLsum - TRsum - BLsum - centerValue
        const RgbaThresholds &alive = l[x + 1];
        // clang-format off
        const uint32_t count =
            (lm1[x].value + lm1[x + 1].value + lm1[x + 2].value + //
               l[x].value                    +   l[x + 2].value + //
             lp1[x].value + lp1[x + 1].value + lp1[x + 2].value)  //
            | alive.value;
        // clang-format on

        const uint32_t p = row[x];
        const int lastR = p >> 11;
        const int lastG = (p >> 6) & 0x1f;
        const int lastB = p & 0x1f;

        const int r = NEXT_EVOLUTION[count >> 15][lastR];
        const int g = NEXT_EVOLUTION[(count >> 10) & 0xf][lastG];
        const int b = NEXT_EVOLUTION[(count >> 4) & 0xf][lastB];

        rowOutput[x] = (r << 11) | (g << 6) | b;
      }

      rowOutput = (rowOutput == outputBuffer0) ? outputBuffer1 : outputBuffer0;
      if (y != startY) {
        memcpy(&buffer[(y - 1) * JAVELIN_DISPLAY_WIDTH], rowOutput,
               2 * JAVELIN_DISPLAY_WIDTH);
      }

      RgbaThresholds *temp = lm1;
      lm1 = l;
      l = lp1;
      lp1 = temp;
    }
    rowOutput = (rowOutput == outputBuffer0) ? outputBuffer1 : outputBuffer0;

    memcpy(&buffer[(endY - 1) * JAVELIN_DISPLAY_WIDTH], rowOutput,
           2 * JAVELIN_DISPLAY_WIDTH);
  }
};

void St7789::St7789Data::RunConwayStep() {
  if (!available) {
    return;
  }

  dirty = true;

  RunConwayStepThreadData *threadData = new RunConwayStepThreadData[2];

  threadData[0].buffer = buffer16;
  threadData[0].startYm1 = JAVELIN_DISPLAY_HEIGHT - 1;
  threadData[0].startY = 0;
  threadData[0].endY = JAVELIN_DISPLAY_HEIGHT / 2;
  memcpy(threadData[0].lastLineBuffer,
         buffer16 + (JAVELIN_DISPLAY_HEIGHT / 2) * JAVELIN_DISPLAY_WIDTH,
         2 * JAVELIN_DISPLAY_WIDTH);

  threadData[1].buffer = buffer16;
  threadData[1].startYm1 = JAVELIN_DISPLAY_HEIGHT / 2 - 1;
  threadData[1].startY = JAVELIN_DISPLAY_HEIGHT / 2;
  threadData[1].endY = JAVELIN_DISPLAY_HEIGHT;
  memcpy(threadData[1].lastLineBuffer, buffer16, 2 * JAVELIN_DISPLAY_WIDTH);

  // Let the previous blit complete before updating the buffer.
  dma4->WaitUntilComplete();

  RunParallel(&RunConwayStepThreadData::ThreadEntryPoint, &threadData[0],
              &RunConwayStepThreadData::ThreadEntryPoint, &threadData[1]);

  delete[] threadData;
}

void St7789::St7789Data::Update() {
  if (!available) {
    return;
  }

  control.Update();

  if (!dirty || dma4->IsBusy()) {
    return;
  }

  ButtonScriptManager::GetInstance().ExecuteScript(
      ButtonScriptId::DISPLAY_OVERLAY);

  dirty = false;

  SendScreenData();
}

void St7789::St7789Data::SendScreenData() const {
  SendCommand(St7789Command::MEMORY_WRITE);

  gpio_put(JAVELIN_DISPLAY_DC_PIN, 1);
  spi_set_format(JAVELIN_DISPLAY_SPI, 16, SPI_CPOL_1, SPI_CPHA_1,
                 SPI_MSB_FIRST);
  dma4->sourceTrigger = buffer32;
}

//---------------------------------------------------------------------------

void St7789::St7789Control::Update() {
  if (dirtyFlag == 0 || dma4->IsBusy()) {
    return;
  }

  if (dirtyFlag & DIRTY_FLAG_SCREEN_ON) {
    SendCommand(data.screenOn ? St7789Command::DISPLAY_ON
                              : St7789Command::DISPLAY_OFF);

#if JAVELIN_DISPLAY_BACKLIGHT_PIN
    gpio_put(JAVELIN_DISPLAY_BACKLIGHT_PIN, data.screenOn);
#endif
  }

  if (dirtyFlag & DIRTY_FLAG_CONTRAST) {
    SendCommand(St7789Command::SET_DISPLAY_BRIGHTNESS, &data.contrast, 1);
  }

  dirtyFlag = 0;
}

//---------------------------------------------------------------------------

void St7789::St7789Availability::UpdateBuffer(TxBuffer &buffer) {
  if (!dirty) {
    return;
  }
  dirty = false;
  buffer.Add(SplitHandlerId::DISPLAY_AVAILABLE, &available, sizeof(bool));
}

void St7789::St7789Availability::OnDataReceived(const void *data,
                                                size_t length) {
  available = *(bool *)data;
}

void St7789::St7789Control::UpdateBuffer(TxBuffer &buffer) {
  if (dirtyFlag == 0) {
    return;
  }
  dirtyFlag = 0;
  buffer.Add(SplitHandlerId::DISPLAY_CONTROL, &data, sizeof(data));
}

void St7789::St7789Control::OnDataReceived(const void *data, size_t length) {
  const St7789ControlTxRxData *controlData =
      (const St7789ControlTxRxData *)data;

  if (this->data.screenOn != controlData->screenOn) {
    this->data.screenOn = controlData->screenOn;
    dirtyFlag |= DIRTY_FLAG_SCREEN_ON;
  }

  if (this->data.contrast != controlData->contrast) {
    this->data.contrast = controlData->contrast;
    dirtyFlag |= DIRTY_FLAG_CONTRAST;
  }
}

void St7789::St7789Control::OnTransmitConnectionReset() {
  dirtyFlag = DIRTY_FLAG_SCREEN_ON | DIRTY_FLAG_CONTRAST;
}

struct St7789TxRxData {
  uint32_t offset;
  uint32_t size;
  uint32_t data[7296 / 4];
};

void St7789::St7789Data::UpdateBuffer(TxBuffer &buffer) {
  if (!available) {
    return;
  }
  if (!dirty && txRxOffset == 0) {
    return;
  }
  if (dma4->IsBusy()) {
    return;
  }

  St7789TxRxData *txData =
      (St7789TxRxData *)buffer.Reserve(sizeof(St7789TxRxData));
  if (txData == nullptr) {
    return;
  }

  if (dirty && txRxOffset == 0) {
    dirty = false;
  }

  txData->offset = txRxOffset;
  txRxOffset += sizeof(St7789TxRxData::data);
  if (txRxOffset > sizeof(buffer32)) {
    txData->size = sizeof(buffer32) - txData->offset;
    txRxOffset = 0;
  } else {
    txData->size = sizeof(St7789TxRxData::data);
  }
  dma5->Copy32(txData->data, buffer8 + txData->offset, txData->size / 4);

  buffer.Add(SplitHandlerId::DISPLAY_DATA, 8 + txData->size);
  dma5->WaitUntilComplete();
}

void St7789::St7789Data::OnDataReceived(const void *data, size_t length) {
  St7789TxRxData *rxData = (St7789TxRxData *)data;

  // Just in case corrupt data comes along
  if (rxData->offset > sizeof(buffer32) ||
      rxData->offset + rxData->size > sizeof(buffer32)) {
    return;
  }

  dma5->Copy32(buffer8 + rxData->offset, rxData->data, rxData->size / 4);

  if (rxData->offset + rxData->size >= sizeof(buffer32)) {
    dirty = true;
  }

  dma5->WaitUntilComplete();
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
  St7789::instances[displayId].Clear();
}

void Display::SetScreenOn(int displayId, bool on) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  St7789::instances[displayId].control.SetScreenOn(on);
}

void Display::SetContrast(int displayId, int contrast) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  St7789::instances[displayId].control.SetContrast(contrast > 255 ? 255
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

  St7789::instances[displayId].dirty = true;
  St7789::instances[displayId].SetPixel(x, y);
}

void Display::DrawLine(int displayId, int x1, int y1, int x2, int y2) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  St7789::instances[displayId].DrawLine(x1, y1, x2, y2);
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
    St7789::instances[displayId].DrawBitmapImage(x, y, width, height, data);
    break;
  case ImageFormat::LUMINANCE8:
    St7789::instances[displayId].DrawLuminanceImage(x, y, width, height, data);
    break;
  case ImageFormat::RGB332:
    St7789::instances[displayId].DrawRgb332Image(x, y, width, height, data);
    break;
  case ImageFormat::RGB565:
    St7789::instances[displayId].DrawRgb565Image(x, y, width, height, data);
    break;
  case ImageFormat::RGB888:
    St7789::instances[displayId].DrawRgb888Image(x, y, width, height, data);
    break;
  case ImageFormat::ALPHA8:
    St7789::instances[displayId].DrawAlpha8Image(x, y, width, height, data);
    break;
  case ImageFormat::ARGB1555:
    St7789::instances[displayId].DrawArgb1555Image(x, y, width, height, data);
    break;
  case ImageFormat::RGBA8888:
    St7789::instances[displayId].DrawRgba8888Image(x, y, width, height, data);
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

  St7789::instances[displayId].DrawLuminanceRange(x, y, width, height, data,
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
  St7789::instances[displayId].DrawText(x, y, font, alignment, text);
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
  St7789::instances[displayId].DrawRect(left, top, right, bottom);
}

void Display::SetDrawColor(int displayId, int color) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  const Color rgb = {
      .r = (uint8_t)(color >> 16),
      .g = (uint8_t)(color >> 8),
      .b = (uint8_t)color,
  };
  St7789::instances[displayId].drawColor = rgb;
  St7789::instances[displayId].drawColor565 = rgb.To565();
}

void Display::SetDrawColorRgb(int displayId, int r, int g, int b) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif
  const Color rgb = {.r = (uint8_t)r, .g = (uint8_t)g, .b = (uint8_t)b};
  St7789::instances[displayId].drawColor = rgb;
  St7789::instances[displayId].drawColor565 = rgb.To565();
}

void Display::DrawEffect(int displayId, int effectId, int parameter) {
#if JAVELIN_SPLIT
  if (displayId < 0 || displayId >= 2) {
    return;
  }
#else
  displayId = 0;
#endif

  switch (effectId) {
  case 0:
    St7789::instances[displayId].RunConwayStep();
    break;
  }
}

//---------------------------------------------------------------------------

#endif // JAVELIN_DISPLAY_DRIVER == 7789

//---------------------------------------------------------------------------
