//---------------------------------------------------------------------------

#include "hid_report_buffer.h"
#include "javelin/clock.h"
#include "javelin/console.h"
#include "pico_split.h"
#include "split_hid_report_buffer.h"
#include "usb_descriptors.h"

#include <string.h>
#include <tusb.h>

//---------------------------------------------------------------------------

constexpr uint32_t FULL_BUFFER_RESET_TIMEOUT = 50;

//---------------------------------------------------------------------------

uint32_t HidReportBufferBase::reportsSentCount[ITF_NUM_TOTAL] = {};

//---------------------------------------------------------------------------

void HidReportBufferBase::SendReport(uint8_t reportId, const uint8_t *data,
                                     size_t length) {
  PumpUntilNotFull();

  const bool triggerSend = startIndex == endIndex;
  if (triggerSend) {

    // Wake up host if we are in suspend mode
    if ((instanceNumber == ITF_NUM_KEYBOARD ||
         instanceNumber == ITF_NUM_CONSOLE) &&
        tud_suspended()) {
      tud_remote_wakeup();
    }

    if (!tud_hid_n_ready(instanceNumber)) {
#if JAVELIN_SPLIT
      if (Split::IsMaster() &&
          Connection::IsPairConnected(PairConnectionId::ACTIVE)) {
        reportsSentCount[instanceNumber]++;
        SplitHidReportBuffer::Add(instanceNumber, reportId, data, length);
      }
#endif
      return;
    }

    if (tud_hid_n_report(instanceNumber, reportId, data, length)) {
      ++endIndex;
      reportsSentCount[instanceNumber]++;
    }
    return;
  }

  const size_t entryIndex = endIndex++ & (NUMBER_OF_ENTRIES - 1);
  Entry *entry = GetEntry(entryIndex);
  entry->length = length;
  entry->reportId = reportId;
  memcpy(entry->data, data, length);
}

//---------------------------------------------------------------------------

void HidReportBufferBase::SendNextReport() {
  // This should never happen!
  if (startIndex == endIndex) {
    return;
  }

  for (;;) {
    ++startIndex;
    if (startIndex == endIndex) {
      return;
    }

    const size_t entryIndex = startIndex & (NUMBER_OF_ENTRIES - 1);
    const Entry *entry = GetEntry(entryIndex);

    reportsSentCount[instanceNumber]++;
    if (tud_hid_n_report(instanceNumber, entry->reportId, entry->data,
                         entry->length)) {
      return;
    }
  }
}

//---------------------------------------------------------------------------

void HidReportBufferBase::PumpUntilNotFull() {
  // Android and Linux do not read 'un-attached' usb endpoints, leading the
  // console to end up in an infinite waiting loop to have a free entry.
  //
  // To protect against this, check if there's no progress for a period of time
  // and reset the buffer if so.
  const size_t startTime = Clock::GetMilliseconds();
  for (;;) {
    tud_task();
    PicoSplit::Update();

    if (!IsFull()) {
      return;
    }

    if (Clock::GetMilliseconds() - startTime > FULL_BUFFER_RESET_TIMEOUT) {
      Reset();
      return;
    }
  }
}

//---------------------------------------------------------------------------

void HidReportBufferBase::PrintInfo() {
  Console::Printf("HID reports sent\n");
  Console::Printf("  Keyboard: %u\n", reportsSentCount[ITF_NUM_KEYBOARD]);
  Console::Printf("  Plover HID: %u\n", reportsSentCount[ITF_NUM_PLOVER_HID]);
  Console::Printf("  Console: %u\n", reportsSentCount[ITF_NUM_CONSOLE]);
}

//---------------------------------------------------------------------------
