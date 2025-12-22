//---------------------------------------------------------------------------

#pragma once
#include <stddef.h>

//---------------------------------------------------------------------------

class PicoSerialPort {
public:
  static void HandleIncomingData();

  static void SendSerialConsole(const void *data, size_t length);
  static void Flush();

  static bool HasActiveSerialConsole();

  static void OnSerialPortDisconnected();

private:
  static bool hasOpenSerialConsole;
};

//---------------------------------------------------------------------------
