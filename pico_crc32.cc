//---------------------------------------------------------------------------

#include "pico_crc32.h"
#include "javelin/crc32.h"
#include "pico_dma.h"
#include "pico_sniff.h"
#include "pico_spinlock.h"

//---------------------------------------------------------------------------

void PicoCrc32::Initialize() {
  // Writing to ROM address 0 seems to work fine as a no-op.
  dma0->destination = 0;

  constexpr PicoDmaSniffControl sniffControl = {
      .enable = true,
      .dmaChannel = 0,
      .calculate = PicoDmaSniffControl::Calculate::BIT_REVERSED_CRC_32,
      .bitReverseOutput = true,
      .bitInvertOutput = true,
  };
  sniff->control = sniffControl;
}

uint32_t PicoCrc32::Hash(const void *data, size_t length, PicoDma *dma) {
#if JAVELIN_THREADS
  spinlock18->Lock();
#endif

  dma->source = data;

  sniff->data = 0xffffffff;

  const bool use32BitTransfer = ((intptr_t(data) | length) & 3) == 0;
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
    dma->control = dmaControl32BitTransfer;
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
    dma->control = dmaControl8BitTransfer;
  }
  dma->countTrigger = length;

  dma->WaitUntilComplete();
  const uint32_t value = sniff->data;

#if JAVELIN_THREADS
  spinlock18->Unlock();
#endif

  return value;
}

//---------------------------------------------------------------------------

uint32_t Crc32::Hash(const void *v, size_t count) {
  return PicoCrc32::Hash(v, count, dma0);
}

//---------------------------------------------------------------------------
