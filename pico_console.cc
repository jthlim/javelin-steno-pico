//---------------------------------------------------------------------------

#include "console_report_buffer.h"
#include "javelin/console.h"
#include "pico_serial_port.h"

//---------------------------------------------------------------------------

static ConnectionId consoleConnectionId = ConnectionId::ACTIVE;

//---------------------------------------------------------------------------

void ConsoleWriter::Write(const char *data, size_t length) {
  if (PicoSerialPort::HasOpenSerialConsole() &&
      (consoleConnectionId == ConnectionId::ACTIVE ||
       consoleConnectionId == ConnectionId::SERIAL_CONSOLE)) {
    PicoSerialPort::SendSerialConsole(data, length);
    if (consoleConnectionId == ConnectionId::ACTIVE) {
      PicoSerialPort::Flush();
    }
  } else {
    ConsoleReportBuffer::instance.SendData((const uint8_t *)data, length);
  }
}

void Console::Flush() { ConsoleReportBuffer::instance.Flush(); }

//---------------------------------------------------------------------------

void ConsoleWriter::SetConnection(ConnectionId connectionId,
                                  uint16_t connectionHandle) {

  if (consoleConnectionId == ConnectionId::SERIAL_CONSOLE) {
    PicoSerialPort::Flush();
  }
  if (connectionId == ConnectionId::ACTIVE) {
    Console::Flush();
  }
  consoleConnectionId = connectionId;
}

//---------------------------------------------------------------------------
