//---------------------------------------------------------------------------

#include "pico_serial_port.h"
#include "javelin/console.h"
#include "javelin/console_input_buffer.h"
#include "javelin/hal/serial_port.h"
#include "javelin/split/split_serial_buffer.h"
#include "javelin/start_console_command_detector.h"
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
  if (PicoSerialPort::HasOpenSerialConsole()) {
    return;
  }
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

bool PicoSerialPort::hasOpenSerialConsole = false;
static StartConsoleCommandDetector commandDetector;

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  if (!dtr) {
    PicoSerialPort::OnSerialPortDisconnected();
  }
}

void PicoSerialPort::OnSerialPortDisconnected() {
  hasOpenSerialConsole = false;
  commandDetector.Reset();
}

void PicoSerialPort::SendSerialConsole(const void *data, size_t length) {
  if (tud_cdc_connected()) {
    size_t written = tud_cdc_write(data, length);
    for (;;) {
      length -= written;
      if (length == 0) {
        break;
      }
      data = (uint8_t *)data + written;
      tud_task();
      written = tud_cdc_write(data, length);
    }
  }
}

void PicoSerialPort::Flush() { tud_cdc_write_flush(); }

void PicoSerialPort::HandleIncomingData() {
  static uint8_t buffer[1024];
  for (uint8_t itf = 0; itf < CFG_TUD_CDC; itf++) {
    while (tud_cdc_n_available(itf)) {
      const uint32_t count = tud_cdc_n_read(itf, buffer, sizeof(buffer));
      if (!hasOpenSerialConsole) {
        const size_t bufferUsedForStartCommand =
            commandDetector.IsStartCommandPresent(buffer, count);
        if (bufferUsedForStartCommand != (size_t)-1) {
          hasOpenSerialConsole = true;
          if (count > bufferUsedForStartCommand) {
            ConsoleInputBuffer::Add(buffer + bufferUsedForStartCommand,
                                    count - bufferUsedForStartCommand,
                                    ConnectionId::SERIAL_CONSOLE);
          }
        }
      } else {
        ConsoleInputBuffer::Add(buffer, count, ConnectionId::SERIAL_CONSOLE);
      }
    }
  }
}

//---------------------------------------------------------------------------
