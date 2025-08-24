//---------------------------------------------------------------------------

#include "pico_serial_port.h"
#include "javelin/hal/serial_port.h"
#include "javelin/split/split_serial_buffer.h"
#include <tusb.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER

void SplitSerialBuffer::SplitSerialBufferData::OnDataReceived(const void *data,
                                                              size_t length) {
  if (tud_cdc_connected()) {
    tud_cdc_write(data, length);
    tud_cdc_write_flush();
  }
}

#endif

//---------------------------------------------------------------------------

void SerialPort::SendData(const void *data, size_t length) {
  if (tud_cdc_connected()) {
    tud_cdc_write(data, length);
    tud_cdc_write_flush();
  } else {
#if JAVELIN_SPLIT
    if (Split::IsMaster()) {
      SplitSerialBuffer::Add(data, length);
    }
#endif
  }
}

//---------------------------------------------------------------------------

void PicoSerialPort::HandleIncomingData() {
  for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++) {
    if (tud_cdc_n_available(itf)) {
      uint8_t buf[64];
      const uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));
      // Do nothing with it.
    }
  }
}

//---------------------------------------------------------------------------