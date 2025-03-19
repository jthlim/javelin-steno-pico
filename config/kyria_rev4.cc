//---------------------------------------------------------------------------

#include "pico_encoder_state.h"
#include <hardware/adc.h>
#include <hardware/spi.h>

//---------------------------------------------------------------------------

uint32_t BUTTON_PIN_MASK = 1 << JAVELIN_ENCODER_BUTTON_PIN;
uint8_t BUTTON_PINS[] = {JAVELIN_ENCODER_BUTTON_PIN};

//---------------------------------------------------------------------------

#if defined(JAVELIN_DISPLAY_BACKLIGHT_PIN)
int readBacklightPin(int testOutput) {
  gpio_init(JAVELIN_DISPLAY_BACKLIGHT_PIN);
  gpio_set_dir(JAVELIN_DISPLAY_BACKLIGHT_PIN, GPIO_OUT);
  gpio_put(JAVELIN_DISPLAY_BACKLIGHT_PIN, testOutput);
  sleep_us(5);
  adc_init();
  gpio_init(JAVELIN_DISPLAY_BACKLIGHT_PIN);
  adc_gpio_init(JAVELIN_DISPLAY_BACKLIGHT_PIN);
  adc_select_input(JAVELIN_DISPLAY_BACKLIGHT_PIN - 26);

  // Dummy read -- first read gives noisy data.
  adc_read();

  // Actual read
  const int value = adc_read();
  gpio_init(JAVELIN_DISPLAY_BACKLIGHT_PIN);
  return value;
}
#endif

bool shouldUseDisplayModule() {
#if !USE_HALCYON_DISPLAY || !defined(JAVELIN_DISPLAY_BACKLIGHT_PIN)
  return false;
#else
  const int value1 = readBacklightPin(0);
  if (value1 < 800 || value1 > 2000) {
    return false;
  }
  const int value2 = readBacklightPin(1);
  if (value2 < 800 || value2 > 2000) {
    return false;
  }
  return true;
#endif
}

static bool IsSpiInitialized(spi_inst_t *instance) {
  return (spi_get_hw(instance)->cr1 & SPI_SSPCR1_SSE_BITS) != 0;
}

bool shouldUseEncoderModule() {
#if !USE_HALCYON_ENCODER
  return false;
#elif defined(JAVELIN_DISPLAY_SPI)
  return !IsSpiInitialized(JAVELIN_DISPLAY_SPI);
#else
  return true;
#endif
}

void preButtonStateInitialize() {
  if (!shouldUseEncoderModule()) {
#if USE_HALCYON_ENCODER
    BUTTON_PINS[0] = JAVELIN_ENCODER_UNUSED_PIN;
    BUTTON_PIN_MASK = 1 << JAVELIN_ENCODER_UNUSED_PIN;
#endif
    PicoEncoderState::SetLocalEncoderCount(1);
  }
}

//---------------------------------------------------------------------------
