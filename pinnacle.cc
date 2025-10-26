//---------------------------------------------------------------------------

#include "pinnacle.h"
#include "javelin/button_script_manager.h"
#include "javelin/clock.h"
#include "javelin/console.h"

#include <hardware/gpio.h>
#include <hardware/spi.h>

//---------------------------------------------------------------------------

#if JAVELIN_POINTER == 0x73a

#if JAVELIN_SPLIT
#define JAVELIN_POINTER_RIGHT_COUNT                                            \
  (JAVELIN_POINTER_COUNT - JAVELIN_POINTER_LEFT_COUNT)
#endif

//---------------------------------------------------------------------------

enum class PinnacleRegister : int {
  FIRMWARE_ID,
  FIRMWARE_VERSION,
  STATUS,
  SYSTEM_CONFIG,
  FEED_CONFIG_1,
  FEED_CONFIG_2,
  FEED_CONFIG_3,
  CALIBRATION_CONFIG,
  PS2_AUX_CONTROL,
  SAMPLE_RATE,
  Z_IDLE,
  Z_SCALER,
  SLEEP_INTERVAL,
  SLEEP_TIMER,
  EMI_ADJUST,
  _RESERVED_0F,
  _RESERVED_10,
  _RESERVED_11,
  PACKET_BYTE_0,
  PACKET_BYTE_1,
  PACKET_BYTE_2,
  PACKET_BYTE_3,
  PACKET_BYTE_4,
  PACKET_BYTE_5,
  PORT_A_GPIO_CONTROL,
  PORT_A_GPIO_DATA,
  PORT_B_GPIO_CONTROL_AND_DATA,
  EXTENDED_REGISTER_VALUE,
  EXTENDED_REGISTER_ADDRESS_HIGH,
  EXTENDED_REGISTER_ADDRESS_LOW,
  EXTENDED_REGISTER_CONTROL,
  PRODUCT_ID,

  // Taken from Cirque example code:
  // https://github.com/cirque-corp/Cirque_Pinnacle_1CA027
  //
  // File:
  // Circular_Trackpad/Single_Pad_Sample_Code/SPI_CurvedOverlay/SPI_CurvedOverlay.ino
  X_AXIS_WIDE_Z_MIN = 0x149,
  Y_AXIS_WIDE_Z_MIN = 0x168,
  ADC_ATTENUATION = 0x187,
};

//---------------------------------------------------------------------------

Pinnacle Pinnacle::instance;

//---------------------------------------------------------------------------

void Pinnacle::InitializeInternal() {
#if JAVELIN_SPLIT
#if JAVELIN_POINTER_LEFT_COUNT == JAVELIN_POINTER_RIGHT_COUNT
  localPointerCount = JAVELIN_POINTER_LEFT_COUNT;
#else
  localPointerCount = Split::IsLeft() ? JAVELIN_POINTER_LEFT_COUNT
                                      : JAVELIN_POINTER_RIGHT_COUNT;
#endif
  localPointerOffset = Split::IsLeft() ? 0 : JAVELIN_POINTER_LEFT_COUNT;
#else // JAVELIN_SPLIT
  localPointerCount = sizeof(POINTER_PINS) / sizeof(*POINTER_PINS);
#endif

  spi_init(JAVELIN_POINTER_SPI, 12'500'000);
  spi_set_format(JAVELIN_POINTER_SPI, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

  gpio_init(JAVELIN_POINTER_MISO_PIN);
  gpio_set_function(JAVELIN_POINTER_MISO_PIN, GPIO_FUNC_SPI);

  gpio_init(JAVELIN_POINTER_MOSI_PIN);
  gpio_set_function(JAVELIN_POINTER_MOSI_PIN, GPIO_FUNC_SPI);

  gpio_init(JAVELIN_POINTER_SCK_PIN);
  gpio_set_function(JAVELIN_POINTER_SCK_PIN, GPIO_FUNC_SPI);

  for (size_t i = 0; i < localPointerCount; ++i) {
    const int chipSelectPin =
        PINNACLE_PINS[localPointerOffset + i].chipSelectPin;
    gpio_init(chipSelectPin);
    gpio_set_dir(chipSelectPin, GPIO_OUT);
    gpio_put(chipSelectPin, 1);

    uint8_t id[2];
    ReadRegisters(chipSelectPin, PinnacleRegister::FIRMWARE_ID, id, 2);
    if (id[0] != 0x7 || id[1] != 0x3a) {
      UninitializeInternal();
      return;
    }

    ClearFlags(chipSelectPin);
    WriteRegister(chipSelectPin, PinnacleRegister::SYSTEM_CONFIG, 0);

    WriteRegister(chipSelectPin, PinnacleRegister::X_AXIS_WIDE_Z_MIN, 4);
    WriteRegister(chipSelectPin, PinnacleRegister::Y_AXIS_WIDE_Z_MIN, 3);
    const int adcAttenuation =
        ReadRegister(chipSelectPin, PinnacleRegister::ADC_ATTENUATION);
    WriteRegister(chipSelectPin, PinnacleRegister::ADC_ATTENUATION,
                  adcAttenuation & 0x3f);

    // Disable GlideExtend, Scroll, Secondary tap, All Taps, Intellimouse
    WriteRegister(chipSelectPin, PinnacleRegister::FEED_CONFIG_2, 0x1f);

    // Feed enable, absolute mode.
    int feedConfig = 3; // Feed enable, absolute mode
    if (PINNACLE_PINS[localPointerOffset + i].invertX) {
      feedConfig |= 0x40;
    }
    if (PINNACLE_PINS[localPointerOffset + i].invertY) {
      feedConfig |= 0x80;
    }

    WriteRegister(chipSelectPin, PinnacleRegister::FEED_CONFIG_1, feedConfig);

    WriteRegister(chipSelectPin, PinnacleRegister::Z_IDLE, 2);
  }

  available = true;
}

void Pinnacle::UninitializeInternal() {
  spi_deinit(JAVELIN_POINTER_SPI);
  gpio_init(JAVELIN_POINTER_MISO_PIN);
  gpio_init(JAVELIN_POINTER_MOSI_PIN);
  gpio_init(JAVELIN_POINTER_SCK_PIN);

  for (size_t i = 0; i < localPointerCount; ++i) {
    const int chipSelectPin =
        PINNACLE_PINS[localPointerOffset + i].chipSelectPin;
    gpio_init(chipSelectPin);
  }
  available = false;
}

//---------------------------------------------------------------------------

void Pinnacle::WriteRegister(int chipSelectPin, PinnacleRegister reg,
                             int value) {
  gpio_put(chipSelectPin, 0);
  if ((int)reg > 0x20) {
    WriteRegister(chipSelectPin, PinnacleRegister::EXTENDED_REGISTER_VALUE,
                  value);
    WriteRegister(chipSelectPin,
                  PinnacleRegister::EXTENDED_REGISTER_ADDRESS_HIGH,
                  int(reg) >> 8);
    WriteRegister(chipSelectPin,
                  PinnacleRegister::EXTENDED_REGISTER_ADDRESS_LOW,
                  int(reg) & 0xff);
    WriteRegister(chipSelectPin, PinnacleRegister::EXTENDED_REGISTER_CONTROL,
                  2);

    while (ReadRegister(chipSelectPin,
                        PinnacleRegister::EXTENDED_REGISTER_CONTROL) != 0) {
      // Busy wait...
    }

    ClearFlags(chipSelectPin);
  } else {
    const uint8_t writeData[2] = {uint8_t(0x80 | (int)reg), uint8_t(value)};
    spi_write_blocking(JAVELIN_POINTER_SPI, writeData, 2);
  }
  gpio_put(chipSelectPin, 1);
}

int Pinnacle::ReadRegister(int chipSelectPin, PinnacleRegister reg) {
  uint8_t result;

  if ((int)reg > 0x20) {
    WriteRegister(chipSelectPin,
                  PinnacleRegister::EXTENDED_REGISTER_ADDRESS_HIGH,
                  int(reg) >> 8);
    WriteRegister(chipSelectPin,
                  PinnacleRegister::EXTENDED_REGISTER_ADDRESS_LOW,
                  int(reg) & 0xff);
    WriteRegister(chipSelectPin, PinnacleRegister::EXTENDED_REGISTER_CONTROL,
                  1);

    while (ReadRegister(chipSelectPin,
                        PinnacleRegister::EXTENDED_REGISTER_CONTROL) != 0) {
      // Busy wait...
    }

    result =
        ReadRegister(chipSelectPin, PinnacleRegister::EXTENDED_REGISTER_VALUE);
    ClearFlags(chipSelectPin);
  } else {
    gpio_put(chipSelectPin, 0);
    const uint8_t writeData[3] = {uint8_t(0xa0 | (int)reg), 0xfb, 0xfb};
    spi_write_blocking(JAVELIN_POINTER_SPI, writeData, 3);

    spi_read_blocking(JAVELIN_POINTER_SPI, 0xfb, &result, 1);
    gpio_put(chipSelectPin, 1);
  }

  return result;
}

void Pinnacle::ReadRegisters(int chipSelectPin, PinnacleRegister startingReg,
                             uint8_t *result, size_t length) {
  gpio_put(chipSelectPin, 0);
  const uint8_t writeData[3] = {uint8_t(0xa0 | (int)startingReg), 0xfc, 0xfc};
  spi_write_blocking(JAVELIN_POINTER_SPI, writeData, 3);

  spi_read_blocking(JAVELIN_POINTER_SPI, 0xfc, result, length);

  gpio_put(chipSelectPin, 1);
}

void Pinnacle::ClearFlags(int chipSelectPin) {
  WriteRegister(chipSelectPin, PinnacleRegister::STATUS, 0);
}

//---------------------------------------------------------------------------

void Pinnacle::UpdateInternal() {
  if (available) {
    for (size_t i = 0; i < localPointerCount; ++i) {
      const int chipSelectPin =
          PINNACLE_PINS[localPointerOffset + i].chipSelectPin;

      // Check for data ready
      const int status = ReadRegister(chipSelectPin, PinnacleRegister::STATUS);
      if ((status & 4) != 0) {
        uint8_t registerData[4];
        ReadRegisters(chipSelectPin, PinnacleRegister::PACKET_BYTE_2,
                      registerData, 4);
        ClearFlags(chipSelectPin);

        Pointer newData;
        newData.x = registerData[0] + ((registerData[2] & 0xf) << 8);
        newData.y = registerData[1] + ((registerData[2] & 0xf0) << 4);
        newData.z = registerData[3] & 0x1f;

        if (newData != data[localPointerOffset + i].pointer) {
          data[localPointerOffset + i].isDirty = true;
          data[localPointerOffset + i].pointer = newData;
#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
          CallScript(localPointerOffset + i, newData);
#endif
        }
      }
    }
  }

#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
  for (size_t i = 0; i < JAVELIN_POINTER_COUNT; ++i) {
    if (data[i].isDirty) {
      data[i].isDirty = false;
      CallScript(i, data[i].pointer);
    }
  }
#endif
}

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
void Pinnacle::OnDataReceived(const void *data, size_t length) {
  const Pointer *pointer = (const Pointer *)data;
  if (Split::IsLeft()) {
    for (size_t i = 0; i < JAVELIN_POINTER_RIGHT_COUNT; ++i) {
      this->data[JAVELIN_POINTER_LEFT_COUNT + i].isDirty = true;
      this->data[JAVELIN_POINTER_LEFT_COUNT + i].pointer = pointer[i];
    }
  } else {
    for (size_t i = 0; i < JAVELIN_POINTER_LEFT_COUNT; ++i) {
      this->data[i].isDirty = true;
      this->data[i].pointer = pointer[i];
    }
  }
}
#endif

#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER
bool Pinnacle::ShouldTransmitData() const {
  for (size_t i = 0; i < localPointerCount; ++i) {
    if (data[localPointerOffset + i].isDirty) {
      return true;
    }
  }
  return false;
}

void Pinnacle::UpdateBuffer(TxBuffer &buffer) {
  if (!ShouldTransmitData()) {
    return;
  }

  if (buffer.Add(SplitHandlerId::POINTER, &data[localPointerOffset].pointer,
                 sizeof(Pointer) * localPointerCount)) {
    for (size_t i = 0; i < localPointerCount; ++i) {
      data[localPointerOffset + i].isDirty = false;
    }
  }
}
#endif

//---------------------------------------------------------------------------

void Pinnacle::CallScript(size_t pointerIndex, const Pointer &pointer) {
#if !defined(JAVELIN_ENCODER_COUNT)
#define JAVELIN_ENCODER_COUNT 0
#endif

#if !defined(JAVELIN_ANALOG_DATA_COUNT)
#define JAVELIN_ANALOG_DATA_COUNT 0
#endif

  const size_t scriptIndex = ButtonScript::GetPointerScriptIndex(
      BUTTON_COUNT, JAVELIN_ANALOG_DATA_COUNT, JAVELIN_ENCODER_COUNT,
      pointerIndex);
  const intptr_t parameters[3] = {pointer.x, pointer.y, pointer.z};

  ButtonScriptManager::GetInstance().ExecuteScriptIndex(
      scriptIndex, Clock::GetMilliseconds(), parameters, 3);
}

//---------------------------------------------------------------------------

void Pinnacle::PrintInfo() {
  Console::Printf("Pinnacle: %s\n",
                  instance.available ? "available" : "not-available");
}

//---------------------------------------------------------------------------

#endif

//---------------------------------------------------------------------------
