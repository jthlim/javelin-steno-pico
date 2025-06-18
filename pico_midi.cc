//---------------------------------------------------------------------------

#include "pico_midi.h"
#include "javelin/console.h"
#include "javelin/hal/midi.h"
#include "javelin/split/split_midi.h"
#include <tusb.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT && !JAVELIN_SPLIT_IS_MASTER

void SplitMidi::SplitMidiData::OnDataReceived(const void *data, size_t length) {
  if (tud_midi_mounted()) {
    tud_midi_stream_write(0, (const uint8_t *)data, length);
  }
}

#endif

//---------------------------------------------------------------------------

void Midi::Send(const uint8_t *data, size_t length) {
  if (tud_midi_mounted()) {
    tud_midi_stream_write(0, data, length);
  } else {
#if JAVELIN_SPLIT
    if (Split::IsMaster()) {
      SplitMidi::Add(data, length);
    }
#endif
  }
}

void PicoMidi::HandleIncomingData() {
  while (tud_midi_available()) {
    uint8_t packet[4];
    tud_midi_packet_read(packet);

    // TODO - Send to script?
  }
}

//---------------------------------------------------------------------------
