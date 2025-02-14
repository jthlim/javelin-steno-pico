//---------------------------------------------------------------------------

#include "pico_crc.h"
#include "pico_dma.h"
#include "pico_sniff.h"
#include "pico_spinlock.h"

//---------------------------------------------------------------------------

void PicoCrc::Initialize() {
  // Writing to ROM address 0 seems to work fine as a no-op.
  dma0->destination = 0;
}

uint32_t PicoCrc::Crc32(const void *data, size_t length) {
#if JAVELIN_THREADS
  spinlock16->Lock();
#endif

  dma0->source = data;

  sniff->data = 0xffffffff;
  constexpr PicoDmaSniffControl sniffControl = {
      .enable = true,
      .dmaChannel = 0,
      .calculate = PicoDmaSniffControl::Calculate::BIT_REVERSED_CRC_32,
      .bitReverseOutput = true,
      .bitInvertOutput = true,
  };
  sniff->control = sniffControl;

  bool use32BitTransfer = ((intptr_t(data) | length) & 3) == 0;
  PicoDmaControl control;
  if (use32BitTransfer) {
    length >>= 2;
    constexpr PicoDmaControl dmaControl32BitTransfer = {
        .enable = true,
        .dataSize = PicoDmaControl::DataSize::WORD,
        .incrementRead = true,
        .incrementWrite = false,
        .chainToDma = 0,
        .transferRequest = PicoDmaTransferRequest::PERMANENT,
        .sniffEnable = true,
    };
    control = dmaControl32BitTransfer;
  } else {
    constexpr PicoDmaControl dmaControl8BitTransfer = {
        .enable = true,
        .dataSize = PicoDmaControl::DataSize::BYTE,
        .incrementRead = true,
        .incrementWrite = false,
        .chainToDma = 0,
        .transferRequest = PicoDmaTransferRequest::PERMANENT,
        .sniffEnable = true,
    };

    control = dmaControl8BitTransfer;
  }
  dma0->count = length;
  dma0->controlTrigger = control;

  dma0->WaitUntilComplete();
  const uint32_t value = sniff->data;

#if JAVELIN_THREADS
  spinlock16->Unlock();
#endif

  return value;
}

uint32_t PicoCrc::Crc16Ccitt(const void *data, size_t length) {
#if JAVELIN_THREADS
  spinlock16->Lock();
#endif

  dma0->source = data;
  dma0->count = length;

  sniff->data = 0xffff;

  constexpr PicoDmaSniffControl sniffControl = {
      .enable = true,
      .dmaChannel = 0,
      .calculate = PicoDmaSniffControl::Calculate::CRC_16_CCITT,
      .bitReverseOutput = false,
      .bitInvertOutput = false,
  };
  sniff->control = sniffControl;

  constexpr PicoDmaControl controlTrigger = {
      .enable = true,
      .dataSize = PicoDmaControl::DataSize::BYTE,
      .incrementRead = true,
      .incrementWrite = false,
      .chainToDma = 0,
      .transferRequest = PicoDmaTransferRequest::PERMANENT,
      .sniffEnable = true,
  };
  dma0->controlTrigger = controlTrigger;

  dma0->WaitUntilComplete();
  uint32_t value = sniff->data;

#if JAVELIN_THREADS
  spinlock16->Unlock();
#endif

  return value;
}

//---------------------------------------------------------------------------

uint32_t Crc32(const void *v, size_t count) { return PicoCrc::Crc32(v, count); }

uint32_t Crc16Ccitt(const void *v, size_t count) {
  return PicoCrc::Crc16Ccitt(v, count);
}

//---------------------------------------------------------------------------
