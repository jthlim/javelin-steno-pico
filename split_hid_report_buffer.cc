//---------------------------------------------------------------------------

#include "split_hid_report_buffer.h"
#include "console_report_buffer.h"
#include "main_report_builder.h"
#include "plover_hid_report_buffer.h"
#include "rp2040_split.h"
#include "usb_descriptors.h"
#include <hardware/watchdog.h>
#include <pico/time.h>
#include <string.h>

//---------------------------------------------------------------------------

#if JAVELIN_SPLIT

//---------------------------------------------------------------------------

SplitHidReportBuffer::SplitHidReportBufferData SplitHidReportBuffer::instance;

//---------------------------------------------------------------------------

void SplitHidReportBuffer::SplitHidReportBufferSize::UpdateBuffer(
    TxBuffer &buffer) {
  HidBufferSize newBufferSize;
  newBufferSize.value = 0;
  newBufferSize.available[ITF_NUM_KEYBOARD] =
      MainReportBuilder::instance.GetAvailableBufferCount();
  newBufferSize.available[ITF_NUM_CONSOLE] =
      ConsoleReportBuffer::instance.GetAvailableBufferCount();
  newBufferSize.available[ITF_NUM_PLOVER_HID] =
      PloverHidReportBuffer::instance.GetAvailableBufferCount();

  if (!dirty && newBufferSize.value == bufferSize.value) {
    return;
  }
  dirty = false;
  bufferSize = newBufferSize;

  buffer.Add(SplitHandlerId::HID_BUFFER_SIZE, &bufferSize, sizeof(bufferSize));
}

void SplitHidReportBuffer::SplitHidReportBufferSize::OnDataReceived(
    const void *data, size_t length) {
  bufferSize = *(HidBufferSize *)data;
}

//---------------------------------------------------------------------------

QueueEntry<SplitHidReportBuffer::EntryData> *
SplitHidReportBuffer::SplitHidReportBufferData::CreateEntry(uint8_t interface,
                                                            uint8_t reportId,
                                                            const uint8_t *data,
                                                            size_t length) {
  QueueEntry<EntryData> *entry = new (length) QueueEntry<EntryData>;
  entry->data.interface = interface;
  entry->data.reportId = reportId;
  entry->data.length = length;
  entry->next = nullptr;
  memcpy(entry->data.data, data, length);
  return entry;
}

//---------------------------------------------------------------------------

void SplitHidReportBuffer::SplitHidReportBufferData::Add(uint8_t interface,
                                                         uint8_t reportId,
                                                         const uint8_t *data,
                                                         size_t length) {
  while (bufferSize.bufferSize.available[interface] == 0) {
    Rp2040Split::Update();
#if JAVELIN_USE_WATCHDOG
    watchdog_update();
#endif
    sleep_us(100);
  }
  bufferSize.bufferSize.available[interface]--;

  QueueEntry<EntryData> *entry = CreateEntry(interface, reportId, data, length);
  AddEntry(entry);
}

void SplitHidReportBuffer::SplitHidReportBufferData::Update() {
  while (head) {
    if (!ProcessEntry(head)) {
      return;
    }

    RemoveHead();
  }
}

bool SplitHidReportBuffer::SplitHidReportBufferData::ProcessEntry(
    const QueueEntry<EntryData> *entry) {
  switch (entry->data.interface) {
  case ITF_NUM_KEYBOARD: {
    auto &reportBuffer = MainReportBuilder::instance.reportBuffer;
    if (reportBuffer.IsFull()) {
      return false;
    }

    reportBuffer.SendReport(entry->data.reportId, entry->data.data,
                            entry->data.length);
    return true;
  }
  case ITF_NUM_CONSOLE: {
    auto &reportBuffer = ConsoleReportBuffer::instance.reportBuffer;
    if (reportBuffer.IsFull()) {
      return false;
    }

    reportBuffer.SendReport(entry->data.reportId, entry->data.data,
                            entry->data.length);
    return true;
  }
  case ITF_NUM_PLOVER_HID: {
    auto &reportBuffer = PloverHidReportBuffer::instance;
    if (reportBuffer.IsFull()) {
      return false;
    }

    reportBuffer.SendReport(entry->data.reportId, entry->data.data,
                            entry->data.length);
    return true;
  }
  }
  // If the interface is unknown, just return true to remove it from the
  // queue.
  return true;
}

//---------------------------------------------------------------------------

void SplitHidReportBuffer::SplitHidReportBufferData::UpdateBuffer(
    TxBuffer &buffer) {
  while (head) {
    if (!buffer.Add(SplitHandlerId::HID_REPORT, &head->data,
                    head->data.length + sizeof(EntryData))) {
      return;
    }

    RemoveHead();
  }
}

void SplitHidReportBuffer::SplitHidReportBufferData::OnDataReceived(
    const void *data, size_t length) {
  const EntryData *entryData = (const EntryData *)data;

  QueueEntry<EntryData> *entry =
      CreateEntry(entryData->interface, entryData->reportId, entryData->data,
                  entryData->length);
  AddEntry(entry);
  bufferSize.dirty = true;
}

//---------------------------------------------------------------------------

#endif // JAVELIN_SPLIT

//---------------------------------------------------------------------------
