cmake_minimum_required(VERSION 3.28)
cmake_policy(SET CMP0076 NEW)

set(NAME javelin-steno-pico)
set(JAVELIN_BOARD "" CACHE STRING "Target board, e.g. \"uni_v4\"")

if ("${JAVELIN_BOARD}" STREQUAL "")
  message(FATAL_ERROR, "Target board (e.g. 'uni_v4') must be specified")
endif()

add_definitions(
  -O2 -g
  -ffunction-sections -fdata-sections
  -fno-devirtualize-speculatively
  -fno-exceptions
  -DJAVELIN_BOARD_CONFIG=\"config/${JAVELIN_BOARD}.h\"
  -DJAVELIN_PLATFORM_PICO_SDK=1
  -DJAVELIN_THREADS=1
  -DJAVELIN_SPLIT_VERSION_INCREMENT=3
  -DNDEBUG=1
  -DPICO_FLASH_SIZE_BYTES=0x1000000
  -DPICO_FLASH_SPI_CLKDIV=4
  -DPICO_MALLOC_PANIC=0
  -DPICO_NO_FPGA_CHECK=1
  -DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
  -DPICO_PANIC_FUNCTION=0
)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if (PICO_PLATFORM STREQUAL "rp2040")
  add_definitions(
    -DJAVELIN_CPU_CORTEX_M0=1
    -DJAVELIN_USE_CUSTOM_POP_COUNT=1
    -DJAVELIN_PICO_PLATFORM=2040
    -DSYS_CLK_MHZ=200
  )
  set (SIZE "$ENV{PICO_TOOLCHAIN_PATH}/bin/arm-none-eabi-size")
elseif (PICO_PLATFORM STREQUAL "rp2350-arm-s")
  add_definitions(
    -flto=auto
    -DJAVELIN_CPU_CORTEX_M33=1
    -DJAVELIN_USE_CUSTOM_POP_COUNT=1
    -DJAVELIN_PICO_PLATFORM=2350
  )
  set (SIZE "$ENV{PICO_TOOLCHAIN_PATH}/bin/arm-none-eabi-size")
elseif (PICO_PLATFORM STREQUAL "rp2350-riscv")
  add_definitions(
    -DJAVELIN_CPU_HAZARD_3=1
    -DJAVELIN_PICO_PLATFORM=2350
  )
  set (SIZE "$ENV{PICO_TOOLCHAIN_PATH}/bin/riscv32-unknown-elf-size")
endif()

project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

add_executable(${NAME})

pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_split_5.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_split_8.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_split_10.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_split_16.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_split_20.pio)
pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/pico_ws2812.pio)

target_sources(${NAME} PUBLIC
  main.cc
  auto_draw.cc
  console_report_buffer.cc
  hid_report_buffer.cc
  libc_overrides.cc
  libc_stubs.cc
  main_report_builder.cc
  main_task.cc
  pico_bindings.cc
  pico_bootloader.cc
  pico_button_state.cc
  pico_clock.cc
  pico_console.cc
  pico_crc32.cc
  pico_encoder_state.cc
  pico_flash.cc
  pico_gpio.cc
  pico_midi.cc
  pico_orthography.cc
  pico_random.cc
  pico_serial_port.cc
  pico_split.cc
  pico_thread.cc
  pico_ws2812.cc
  pinnacle.cc
  plover_hid_report_buffer.cc
  split_hid_report_buffer.cc
  ssd1306.cc
  ssd1306_paper_tape.cc
  ssd1306_steno_layout.cc
  st7789.cc
  st7789_paper_tape.cc
  st7789_steno_layout.cc
  usb_descriptors.cc

  javelin/asset_manager.cc
  javelin/base64.cc
  javelin/bit.cc
  javelin/button_script.cc
  javelin/button_script_manager.cc
  javelin/console.cc
  javelin/console_input_buffer.cc
  javelin/container/list.cc
  javelin/crc32.cc
  javelin/dictionary/compact_map_dictionary.cc
  javelin/dictionary/debug_dictionary.cc
  javelin/dictionary/dictionary.cc
  javelin/dictionary/dictionary_definition.cc
  javelin/dictionary/dictionary_list.cc
  javelin/dictionary/emily_symbols_dictionary.cc
  javelin/dictionary/full_map_dictionary.cc
  javelin/dictionary/invalid_dictionary.cc
  javelin/dictionary/jeff_numbers_dictionary.cc
  javelin/dictionary/jeff_phrasing_dictionary.cc
  javelin/dictionary/jeff_phrasing_dictionary_data.cc
  javelin/dictionary/jeff_show_stroke_dictionary.cc
  javelin/dictionary/orthospelling_data.cc
  javelin/dictionary/orthospelling_dictionary.cc
  javelin/dictionary/reverse_auto_suffix_dictionary.cc
  javelin/dictionary/reverse_map_dictionary.cc
  javelin/dictionary/reverse_prefix_dictionary.cc
  javelin/dictionary/reverse_suffix_dictionary.cc
  javelin/dictionary/unicode_dictionary.cc
  javelin/dictionary/user_dictionary.cc
  javelin/dictionary/wrapped_dictionary.cc
  javelin/engine_add_translation_mode.cc
  javelin/engine_binding.cc
  javelin/engine_console_mode.cc
  javelin/engine_normal_mode.cc
  javelin/engine.cc
  javelin/flash.cc
  javelin/font/monochrome/data/default.cc
  javelin/font/monochrome/data/dos.cc
  javelin/font/monochrome/data/huge_digits.cc
  javelin/font/monochrome/data/large.cc
  javelin/font/monochrome/data/large_digits.cc
  javelin/font/monochrome/data/medium_digits.cc
  javelin/font/monochrome/data/small_digits.cc
  javelin/font/monochrome/font.cc
  javelin/hal/ble.cc
  javelin/hal/bootloader.cc
  javelin/hal/connection.cc
  javelin/hal/display.cc
  javelin/hal/gpio.cc
  javelin/hal/infrared.cc
  javelin/hal/mouse.cc
  javelin/hal/power.cc
  javelin/hal/rgb.cc
  javelin/hal/sound.cc
  javelin/hal/usb_status.cc
  javelin/host_layout.cc
  javelin/key.cc
  javelin/key_code.cc
  javelin/key_press_parser.cc
  javelin/mem.cc
  javelin/orthography.cc
  javelin/pattern.cc
  javelin/pattern_component.cc
  javelin/pool_allocate.cc
  javelin/processor/all_up.cc
  javelin/processor/first_up.cc
  javelin/processor/gemini.cc
  javelin/processor/jeff_modifiers.cc
  javelin/processor/paper_tape.cc
  javelin/processor/plover_hid.cc
  javelin/processor/procat.cc
  javelin/processor/processor.cc
  javelin/processor/processor_list.cc
  javelin/processor/repeat.cc
  javelin/processor/tx_bolt.cc
  javelin/random.cc
  javelin/script.cc
  javelin/script_byte_code.cc
  javelin/script_storage.cc
  javelin/segment.cc
  javelin/segment_builder.cc
  javelin/split/split.cc
  javelin/split/split_console.cc
  javelin/split/split_key_state.cc
  javelin/split/split_midi.cc
  javelin/split/split_power_override.cc
  javelin/split/split_serial_buffer.cc
  javelin/split/split_usb_status.cc
  javelin/split/split_version.cc
  javelin/state.cc
  javelin/steno_key_code.cc
  javelin/steno_key_code_buffer.cc
  javelin/steno_key_code_buffer_functions.cc
  javelin/steno_key_code_emitter.cc
  javelin/steno_key_state.cc
  javelin/str.cc
  javelin/stroke.cc
  javelin/stroke_history.cc
  javelin/stroke_list_parser.cc
  javelin/system.cc
  javelin/timer_manager.cc
  javelin/unicode.cc
  javelin/utf8_pointer.cc
  javelin/varint_writer.cc
  javelin/word_list.cc
  javelin/wpm_tracker.cc
  javelin/writer.cc
)

# Add the local directory so tinyusb can find tusb_config.h
target_include_directories(${NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${NAME}
  hardware_adc
  hardware_flash
  hardware_gpio
  hardware_i2c
  hardware_pio
  hardware_pwm
  hardware_spi
  pico_bootrom
  pico_multicore
  pico_stdlib
  tinyusb_device
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

add_custom_command(TARGET ${NAME} POST_BUILD
  COMMAND ${SIZE} javelin-steno-pico.elf
)