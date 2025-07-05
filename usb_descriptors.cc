//---------------------------------------------------------------------------

#include "usb_descriptors.h"
#include "class/hid/hid.h"
#include <hardware/flash.h>
#include <tusb.h>

#include JAVELIN_BOARD_CONFIG

//---------------------------------------------------------------------------

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define USB_PID                                                                \
  (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) |           \
   _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

//---------------------------------------------------------------------------
// Device Descriptors
//---------------------------------------------------------------------------

const tusb_desc_device_t DEVICE_DESCRIPTOR = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = VENDOR_ID,
    .idProduct = USB_PID,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
extern "C" const uint8_t *tud_descriptor_device_cb(void) {
  return (const uint8_t *)&DEVICE_DESCRIPTOR;
}

//---------------------------------------------------------------------------
// HID Report Descriptor
//---------------------------------------------------------------------------

// clang-format off
const uint8_t keyboardReportDescriptor[] = {
    // Keyboard Page
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
      // Keyboard
      HID_REPORT_ID(KEYBOARD_PAGE_REPORT_ID)
      HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),

      // 1 input byte. Bitmap of modifiers.
      HID_USAGE_MIN(224),
      HID_USAGE_MAX(231),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(8),
      HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // 13 input bytes. Bitmap of keys.
      HID_USAGE_MIN(0),
      HID_USAGE_MAX(103),
      HID_REPORT_COUNT(104),
      // HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      // 2 input bytes. Keyboard page array.
      HID_USAGE_MIN(0x00),
      HID_USAGE_MAX(0xff),
      HID_LOGICAL_MIN(0x00),
      HID_LOGICAL_MAX_N(0xff, 2),
      HID_REPORT_COUNT(2),
      HID_REPORT_SIZE(8),
      HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE),

    // 1 output byte. LED Indicator
    HID_USAGE_PAGE(HID_USAGE_PAGE_LED),
      HID_USAGE_MIN(1),
      HID_USAGE_MAX(5),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(5),
      HID_REPORT_SIZE(1),
      HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
      // Padding
      HID_REPORT_COUNT(1),
      HID_REPORT_SIZE(3),
      HID_OUTPUT(HID_CONSTANT),

    HID_COLLECTION_END,

    // --------------------------

    // Consumer Page
    HID_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER),
    HID_USAGE(HID_USAGE_CONSUMER_CONTROL),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

    // 8 input bytes. Consumer Page
      HID_REPORT_ID(CONSUMER_PAGE_REPORT_ID)
      HID_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER),
      HID_USAGE_MIN(0xb0),
      HID_USAGE_MAX(0xfd),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(62),
      HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

      HID_USAGE_MIN(0x6f),
      HID_USAGE_MAX(0x70),
      // HID_LOGICAL_MIN(0),
      // HID_LOGICAL_MAX(1),
      HID_REPORT_COUNT(2),
      // HID_REPORT_SIZE(1),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

    HID_COLLECTION_END,

    // --------------------------

    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
    HID_USAGE(HID_USAGE_DESKTOP_MOUSE),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),

      HID_REPORT_ID(MOUSE_PAGE_REPORT_ID)
      HID_USAGE_PAGE(HID_USAGE_DESKTOP_POINTER),
      HID_COLLECTION(HID_COLLECTION_PHYSICAL),

        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
        HID_USAGE_MIN(1),
        HID_USAGE_MAX(0x20),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(32),
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),

        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
        HID_USAGE(HID_USAGE_DESKTOP_X),
        HID_USAGE(HID_USAGE_DESKTOP_Y),
        HID_USAGE(HID_USAGE_DESKTOP_WHEEL),
        HID_LOGICAL_MIN_N(-32768, 2),
        HID_LOGICAL_MAX_N(32767, 2),
        HID_REPORT_SIZE(16),
        HID_REPORT_COUNT(3),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),

        HID_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER),
        HID_USAGE_N(HID_USAGE_CONSUMER_AC_PAN, 2),
        HID_REPORT_COUNT(1),
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_RELATIVE),

      HID_COLLECTION_END,
    HID_COLLECTION_END,
};



const uint8_t consoleReportDescriptor[] = {
    // Console
    0x06, 0x31, 0xFF, // HID_USAGE_PAGE(0xff31)
    HID_USAGE(0x0074),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    // Send
      HID_USAGE(0x75),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(255),
      HID_REPORT_COUNT(64),
      HID_REPORT_SIZE(8),
      HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    // Receive
      HID_USAGE(0x76),
      HID_LOGICAL_MIN(0),
      HID_LOGICAL_MAX(255),
      HID_REPORT_COUNT(64),
      HID_REPORT_SIZE(8),
      HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

const uint8_t ploverHidReportDescriptor[] = {
     0x06, 0x50, 0xff,              // UsagePage (65360)
     0x0a, 0x56, 0x4c,              // Usage (19542)
     0xa1, 0x02,                    // Collection (Logical)
     0x85, 0x50,                    //     ReportID (80)
     0x25, 0x01,                    //     LogicalMaximum (1)
     0x75, 0x01,                    //     ReportSize (1)
     0x95, 0x40,                    //     ReportCount (64)
     0x05, 0x0a,                    //     UsagePage (ordinal)
     0x19, 0x00,                    //     UsageMinimum (Ordinal(0))
     0x29, 0x3f,                    //     UsageMaximum (Ordinal(63))
     0x81, 0x02,                    //     Input (Variable)
     0xc0,                          // EndCollection
};
// clang-format on

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
extern "C" const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance) {
  switch (instance) {
  case ITF_NUM_KEYBOARD:
    return keyboardReportDescriptor;

  case ITF_NUM_CONSOLE:
    return consoleReportDescriptor;

  case ITF_NUM_PLOVER_HID:
    return ploverHidReportDescriptor;

  default:
    __builtin_unreachable();
  }
}

//---------------------------------------------------------------------------
// Configuration Descriptor
//---------------------------------------------------------------------------

#define CONFIG_TOTAL_LEN                                                       \
  (TUD_CONFIG_DESC_LEN + 2 * TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN +       \
   TUD_CDC_DESC_LEN + TUD_MIDI_DESC_LEN)

#define EPNUM_KEYBOARD 0x81

#define EPNUM_PLOVER_HID 0x82

#define EPNUM_CONSOLE_OUT 0x03
#define EPNUM_CONSOLE_IN 0x83

#define EPNUM_CDC_NOTIF 0x84
#define EPNUM_CDC_OUT 0x05
#define EPNUM_CDC_IN 0x85

#define EPNUM_MIDI_OUT 0x06
#define EPNUM_MIDI_IN 0x86

const uint8_t MAIN_CONFIGURATION_DESCRIPTOR[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
                          JAVELIN_USB_MILLIAMPS),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_KEYBOARD, 0, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(keyboardReportDescriptor), EPNUM_KEYBOARD,
                       CFG_KEYBOARD_BUFSIZE, 1),

    // HID Input & Output descriptor
    // Interface number, string index, protocol, report descriptor len,
    // EP OUT & IN address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_CONSOLE, 0, HID_ITF_PROTOCOL_NONE,
                             sizeof(consoleReportDescriptor), EPNUM_CONSOLE_OUT,
                             EPNUM_CONSOLE_IN, CFG_CONSOLE_BUFSIZE, 1),

    // Interface number, string index, protocol, report descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_PLOVER_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(ploverHidReportDescriptor), EPNUM_PLOVER_HID,
                       CFG_PLOVER_HID_EP_BUFSIZE, 1),

    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT,
                       EPNUM_CDC_IN, 64),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64),
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
extern "C" const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
  return MAIN_CONFIGURATION_DESCRIPTOR;
}

//---------------------------------------------------------------------------
// String Descriptors
//---------------------------------------------------------------------------

// array of pointer to string descriptors
const char *const STRING_DESCRIPTORS[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    MANUFACTURER_NAME,          // 1: Manufacturer
    PRODUCT_NAME,               // 2: Product
    "",                         // 3: Serials, should use chip ID
};

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
extern "C" const uint16_t *tud_descriptor_string_cb(uint8_t index,
                                                    uint16_t langid) {
  size_t offset;
  static uint16_t buffer[32];

  switch (index) {
  case 0:
    memcpy(&buffer[1], STRING_DESCRIPTORS[0], 2);
    offset = 1;
    break;

  case 3: {
    uint8_t id[8];
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_get_unique_id(id);
    restore_interrupts(interrupts);
    for (size_t i = 0; i < 8; ++i) {
      buffer[i * 2 + 1] = "0123456789abcdef"[id[i] >> 4];
      buffer[i * 2 + 2] = "0123456789abcdef"[id[i] & 0xf];
    }
    offset = 16;
    break;
  }
  default:
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
    if (index >= sizeof(STRING_DESCRIPTORS) / sizeof(*STRING_DESCRIPTORS)) {
      return nullptr;
    }

    const char *str = STRING_DESCRIPTORS[index];

    // Cap at max char
    offset = strlen(str);
    if (offset > 31) {
      offset = 31;
    }

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < offset; i++) {
      buffer[1 + i] = str[i];
    }
    break;
  }

  // first byte is length (including header), second byte is string type
  buffer[0] = (TUSB_DESC_STRING << 8) | (2 * offset + 2);

  return buffer;
}
