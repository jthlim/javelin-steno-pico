//---------------------------------------------------------------------------

#include "console_report_buffer.h"
#include "javelin/console.h"
#include "javelin/split/split_serial_buffer.h"
#include "javelin/split/split_usb_status.h"
#include "pico_serial_port.h"

//---------------------------------------------------------------------------

static ConnectionId consoleConnectionId = ConnectionId::ACTIVE;

//---------------------------------------------------------------------------

void ConsoleWriter::Write(const char *data, size_t length) {
  switch (consoleConnectionId) {
  case ConnectionId::ACTIVE:
    if (SplitUsbStatus::GetLocalUsbStatus().IsSerialConsoleActive()) {
      PicoSerialPort::SendSerialConsole(data, length);
      return;
    }

#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
    if (SplitUsbStatus::GetRemoteUsbStatus().IsSerialConsoleActive()) {
      SplitSerialBuffer::Add(data, length);
      return;
    }
#endif
    [[fallthrough]];

  case ConnectionId::USB:
  case ConnectionId::USB_PAIR:
    ConsoleReportBuffer::instance.SendData((const uint8_t *)data, length);
    return;

  case ConnectionId::BLE:
  case ConnectionId::BLE_CONSOLE:
    return;

  case ConnectionId::SERIAL_CONSOLE:
    if (SplitUsbStatus::GetLocalUsbStatus().IsSerialConsoleActive()) {
      PicoSerialPort::SendSerialConsole(data, length);
    }
    return;

  case ConnectionId::SERIAL_CONSOLE_PAIR:
#if JAVELIN_SPLIT && JAVELIN_SPLIT_IS_MASTER
    SplitSerialBuffer::Add(data, length);
#endif
    return;
  }
}

void Console::Flush() {
  switch (consoleConnectionId) {
  case ConnectionId::ACTIVE:
    if (SplitUsbStatus::GetLocalUsbStatus().IsSerialConsoleActive()) {
      PicoSerialPort::Flush();
      return;
    }
    [[fallthrough]];

  case ConnectionId::USB:
  case ConnectionId::USB_PAIR:
    ConsoleReportBuffer::instance.Flush();
    return;

  case ConnectionId::BLE:
  case ConnectionId::BLE_CONSOLE:
    return;

  case ConnectionId::SERIAL_CONSOLE:
    if (SplitUsbStatus::GetLocalUsbStatus().IsSerialConsoleActive()) {
      PicoSerialPort::Flush();
    }
    return;

  case ConnectionId::SERIAL_CONSOLE_PAIR:
    return;
  }
}

//---------------------------------------------------------------------------

void ConsoleWriter::SetConnection(ConnectionId connectionId,
                                  uint16_t connectionHandle) {

  if (consoleConnectionId != connectionId) {
    Console::Flush();
  }
  consoleConnectionId = connectionId;
}

//---------------------------------------------------------------------------
