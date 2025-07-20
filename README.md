# Raspberry Pi pico-sdk bindings for javelin-steno.

## Build Instructions

1. Ensure that you have a recent Arm GNU Toolchain (>=12) and CMake (>=4) installed.

2. Install [pico-sdk](https://github.com/raspberrypi/pico-sdk) and ensure that
   you can build the examples.

3. Clone [javelin-steno](https://github.com/jthlim/javelin-steno) repository.

4. Clone this repository.

5. From within this repository, link the javelin-steno repository:

```
> ln -s <path-to-javelin-steno> javelin
```

6. Standard CMake, with '-D JAVELIN_BOARD=xxx'

```
> mkdir build
> cd build
> cmake .. -D JAVELIN_BOARD=uni_v4
> make
```

   For rp2350 boards, add `-DPICO_PLATFORM=rp2350-arm-s`, e.g.,
```
> cmake .. -DJAVELIN_BOARD=starboard_rp2350 -DPICO_PLATFORM=rp2350-arm-s
```


You should now have a uf2 file that can be copied to the device.

## Using javelin-steno-pico with your own hardware.

This process involves uploading all of the data to the keyboard, then
overwriting the bit of the firmware that is specific to interfacing with
your keyboard's hardware.

1. First, use the online firmware builder to build a firmware that matches
   your chip. It is recommended to use starboard rp2040 or rp2350. Upload
   this to your board and confirm that you can connect to the web tools
   and do a lookup.

2. Copy one of the `config/*.h` files to create a new configuration.

   Example configurations:

   Type    | Direct Wired     | Matrix
   --------|------------------|---------
   Unibody | starboard_rp2040 | uni_v4
   Split   | crkbd_v4         | crkbd_v3

3. Update your pin/button configuration there.

4. `JAVELIN_SCRIPT_CONFIGURATION` inside the config file is a JSON that enables
   the online web tools to provide a visual editor configuration.

5. Build the uf2 firmware and upload it to your keyboard.

6. Use the web Script tool to upload a configuration to your keyboard. This
   maps buttons to steno keys/other actions.

If you use a split keyboard, then you will need to repeat this with a *_pair.h
firmware for the other side.

# Contributions

This project is a snapshot of internal repositories and is not accepting any
pull requests.

# Terms

This code is distributed under [PolyForm Noncommercial license](LICENSE.txt).
For commercial use, please [contact](mailto:jeff@lim.au) me.

# Troubleshooting

When building, if you receive a message like:

```
error: uinitialized const member in 'const char []'
```

This means that you're running an older version of the toolchain that has a
compiler bug. Update to a newer version at
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
