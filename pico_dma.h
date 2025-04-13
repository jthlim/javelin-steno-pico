//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_PICO_PLATFORM == 2350

enum class PicoDmaTransferRequest : uint32_t {
  PIO0_TX0 = 0,
  PIO0_TX1,
  PIO0_TX2,
  PIO0_TX3,
  PIO0_RX0,
  PIO0_RX1,
  PIO0_RX2,
  PIO0_RX3,
  PIO1_TX0,
  PIO1_TX1,
  PIO1_TX2,
  PIO1_TX3,
  PIO1_RX0,
  PIO1_RX1,
  PIO1_RX2,
  PIO1_RX3,
  PIO2_TX0,
  PIO2_TX1,
  PIO2_TX2,
  PIO2_TX3,
  PIO2_RX0,
  PIO2_RX1,
  PIO2_RX2,
  PIO2_RX3,
  SPI0_TX,
  SPI0_RX,
  SPI1_TX,
  SPI1_RX,
  UART0_TX,
  UART0_RX,
  UART1_TX,
  UART1_RX,
  PWM_WRAP0,
  PWM_WRAP1,
  PWM_WRAP2,
  PWM_WRAP3,
  PWM_WRAP4,
  PWM_WRAP5,
  PWM_WRAP6,
  PWM_WRAP7,
  PWM_WRAP8,
  PWM_WRAP9,
  PWM_WRAP10,
  PWM_WRAP11,
  I2C0_TX,
  I2C0_RX,
  I2C1_TX,
  I2C1_RX,
  ADC,
  XIP_STREAM,
  XIP_QMITX,
  XIP_QMIRX,
  XIP_HSTX,
  XIP_CORESIGHT,
  XIP_SHA256,
  TIMER_0 = 0x3b,
  TIMER_1 = 0x3c,
  TIMER_2 = 0x3d,
  TIMER_3 = 0x3e,
  PERMANENT = 0x3f,
};

struct PicoDmaControl {

  enum class DataSize : uint32_t {
    BYTE = 0,
    HALF_WORD = 1,
    WORD = 2,
  };

  uint32_t enable : 1;
  uint32_t highPriority : 1;
  DataSize dataSize : 2;
  uint32_t incrementRead : 1;
  uint32_t incrementReadReverse : 1;
  uint32_t incrementWrite : 1;
  uint32_t incrementWriteReverse : 1;
  uint32_t ringSizeShift : 4;
  uint32_t ringSel : 1;

  // Set to the channel being used to disable chaining.
  uint32_t chainToDma : 4;

  PicoDmaTransferRequest transferRequest : 6;
  uint32_t quietIrq : 1;
  uint32_t bswap : 1;
  uint32_t sniffEnable : 1;
  uint32_t busy : 1;
  uint32_t _reserved27 : 2;
  uint32_t writeError : 1;
  uint32_t readError : 1;
  uint32_t ahbError : 1;

  void operator=(const PicoDmaControl &control) volatile {
    *(volatile uint32_t *)this = *(uint32_t *)&control;
  }
};
static_assert(sizeof(PicoDmaControl) == 4, "Unexpected DmaControl size");

#elif JAVELIN_PICO_PLATFORM == 2040

enum class PicoDmaTransferRequest : uint32_t {
  PIO0_TX0 = 0,
  PIO0_TX1,
  PIO0_TX2,
  PIO0_TX3,
  PIO0_RX0,
  PIO0_RX1,
  PIO0_RX2,
  PIO0_RX3,
  PIO1_TX0,
  PIO1_TX1,
  PIO1_TX2,
  PIO1_TX3,
  PIO1_RX0,
  PIO1_RX1,
  PIO1_RX2,
  PIO1_RX3,
  SPI0_TX,
  SPI0_RX,
  SPI1_TX,
  SPI1_RX,
  UART0_TX,
  UART0_RX,
  UART1_TX,
  UART1_RX,
  PWM_WRAP0,
  PWM_WRAP1,
  PWM_WRAP2,
  PWM_WRAP3,
  PWM_WRAP4,
  PWM_WRAP5,
  PWM_WRAP6,
  PWM_WRAP7,
  I2C0_TX,
  I2C0_RX,
  I2C1_TX,
  I2C1_RX,
  ADC,
  XIP_STREAM,
  XIP_SSITX,
  XIP_SSIRX,
  TIMER_0 = 0x3b,
  TIMER_1 = 0x3c,
  TIMER_2 = 0x3d,
  TIMER_3 = 0x3e,
  PERMANENT = 0x3f,
};

struct PicoDmaControl {

  enum class DataSize : uint32_t {
    BYTE = 0,
    HALF_WORD = 1,
    WORD = 2,
  };

  uint32_t enable : 1;
  uint32_t highPriority : 1;
  DataSize dataSize : 2;
  uint32_t incrementRead : 1;
  uint32_t incrementWrite : 1;
  uint32_t ringSizeShift : 4;
  uint32_t ringSel : 1;

  // Set to the channel being used to disable chaining.
  uint32_t chainToDma : 4;

  PicoDmaTransferRequest transferRequest : 6;
  uint32_t quietIrq : 1;
  uint32_t bswap : 1;
  uint32_t sniffEnable : 1;
  uint32_t busy : 1;
  uint32_t _reserved25 : 4;
  uint32_t writeError : 1;
  uint32_t readError : 1;
  uint32_t ahbError : 1;

  void operator=(const PicoDmaControl &control) volatile {
    *(volatile uint32_t *)this = *(uint32_t *)&control;
  }
};
static_assert(sizeof(PicoDmaControl) == 4, "Unexpected DmaControl size");

#else
#error Unsupported platform
#endif

//---------------------------------------------------------------------------

struct PicoDmaIrqControl {
  struct Irq {
    volatile uint32_t enableMask;
    volatile uint32_t forceMask;
    volatile uint32_t status;

    uint32_t AckAllIrqs() {
      const uint32_t mask = status;
      status = mask;
      return mask;
    }
    void AckIrq(int dmaChannel) { status = (1 << dmaChannel); }
    void EnableIrq(int dmaChannel) { enableMask |= (1 << dmaChannel); }
  };

  volatile uint32_t status;
  Irq irq0;
  uint32_t _unused10;
  Irq irq1;
};

static PicoDmaIrqControl *const dmaIrqControl = (PicoDmaIrqControl *)0x50000400;

//---------------------------------------------------------------------------

struct PicoDmaAbort {
  volatile uint32_t value;

  void Abort(int channelIndex) { value = (1 << channelIndex); }
};

#if JAVELIN_PICO_PLATFORM == 2350
static PicoDmaAbort *const dmaAbort = (PicoDmaAbort *)0x50000464;
#elif JAVELIN_PICO_PLATFORM == 2040
static PicoDmaAbort *const dmaAbort = (PicoDmaAbort *)0x50000444;
#else
#error Unsupported platform
#endif

//---------------------------------------------------------------------------

struct PicoDma {
  const volatile void *volatile source;
  const volatile void *volatile destination;
  volatile uint32_t count;
  volatile PicoDmaControl controlTrigger;

  // Alias 1
  union {
    volatile PicoDmaControl control;
    volatile PicoDmaControl controlAlias1;
  };
  const volatile void *volatile sourceAlias1;
  const volatile void *volatile destinationAlias1;
  volatile uint32_t countTrigger;

  // Alias 2
  volatile PicoDmaControl controlAlias2;
  volatile uint32_t countAlias2;
  const volatile void *volatile sourceAlias2;
  const volatile void *volatile destinationTrigger;

  // Alias 3
  volatile PicoDmaControl controlAlias3;
  const volatile void *volatile destinationAlias3;
  volatile uint32_t countAlias3;
  const volatile void *volatile sourceTrigger;

  inline bool IsBusy() const { return control.busy; }

  inline void WaitUntilComplete() const {
    while (IsBusy()) {
    }
  }

  void Abort() {
    dmaAbort->Abort(this - (PicoDma *)0x50000000);
    WaitUntilComplete();
  }

  inline void Copy16(void *dest, const void *source, size_t transferCount);
  inline void Copy32(void *dest, const void *source, size_t transferCount);
};

static PicoDma *const dma0 = (PicoDma *)0x50000000;
static PicoDma *const dma1 = (PicoDma *)0x50000040;
static PicoDma *const dma2 = (PicoDma *)0x50000080;
static PicoDma *const dma3 = (PicoDma *)0x500000c0;
static PicoDma *const dma4 = (PicoDma *)0x50000100;
static PicoDma *const dma5 = (PicoDma *)0x50000140;
static PicoDma *const dma6 = (PicoDma *)0x50000180;
static PicoDma *const dma7 = (PicoDma *)0x500001c0;
static PicoDma *const dma8 = (PicoDma *)0x50000200;
static PicoDma *const dma9 = (PicoDma *)0x50000240;
static PicoDma *const dma10 = (PicoDma *)0x50000280;
static PicoDma *const dma11 = (PicoDma *)0x500002c0;

#if JAVELIN_PICO_PLATFORM == 2350
static volatile uint32_t *const dmaTimer0 = (uint32_t *)0x50000440;
static volatile uint32_t *const dmaTimer1 = (uint32_t *)0x50000444;
static volatile uint32_t *const dmaTimer2 = (uint32_t *)0x50000448;
static volatile uint32_t *const dmaTimer3 = (uint32_t *)0x5000044c;
#elif JAVELIN_PICO_PLATFORM == 2040
// Upper 16 bits = numerator.
// Lower 16 bits = denominator.
static volatile uint32_t *const dmaTimer0 = (uint32_t *)0x50000420;
static volatile uint32_t *const dmaTimer1 = (uint32_t *)0x50000424;
static volatile uint32_t *const dmaTimer2 = (uint32_t *)0x50000428;
static volatile uint32_t *const dmaTimer3 = (uint32_t *)0x5000042c;
#else
#error Unsupported Platform
#endif

//---------------------------------------------------------------------------

inline void PicoDma::Copy16(void *d, const void *s, size_t transferCount) {
  source = s;
  destination = d;
  count = transferCount;
  const PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::HALF_WORD,
      .incrementRead = true,
      .incrementWrite = true,
      .chainToDma = uint32_t(this - dma0),
      .transferRequest = PicoDmaTransferRequest::PERMANENT,
      .sniffEnable = false,
  };
  controlTrigger = dmaControl;
}

inline void PicoDma::Copy32(void *d, const void *s, size_t transferCount) {
  source = s;
  destination = d;
  count = transferCount;
  const PicoDmaControl dmaControl = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::WORD,
      .incrementRead = true,
      .incrementWrite = true,
      .chainToDma = uint32_t(this - dma0),
      .transferRequest = PicoDmaTransferRequest::PERMANENT,
      .sniffEnable = false,
  };
  controlTrigger = dmaControl;
}

//---------------------------------------------------------------------------
