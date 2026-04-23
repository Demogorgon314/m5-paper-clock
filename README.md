# M5Paper Ink Clock

A horizontal e-ink clock for M5Paper-style hardware built with PlatformIO and
the M5EPD Arduino library.

## Features

- Wi-Fi setup flow on first boot
- Password entry with show/hide toggle
- RTC sync from NTP with configurable timezone
- Auto-connect on boot and auto-enter clock view after successful sync
- Large segmented clock layout optimized for landscape mode
- Temperature and humidity from the onboard SHT30 sensor
- Internal SPIFFS-backed CJK TTF for UI text and Chinese stock names
- G38 center button:
  - short press: full refresh
  - long press: enter settings

## Project Layout

- `src/ClockApp.*` - app state, pages, input handling, and rendering
- `src/SegmentRenderer.*` - segmented digit drawing
- `src/ConnectivityService.*` - Wi-Fi and NTP sync
- `src/SettingsStore.*` - persisted settings
- `src/SensorService.*` - SHT30 temperature and humidity access
- `web/*` - Chrome Web Serial USB configuration page
- `include/logic/*` - small UI and segment helpers
- `test/test_logic/test_main.cpp` - native logic tests

## Build

Builds use the checked-in holiday data directly, so day-to-day compilation works
offline without any pre-build network step.

```bash
/Users/wangkai/.platformio/penv/bin/platformio run -e m5stack-fire
```

The bundled `data/cjk.ttf` font lives on SPIFFS, so after changing the font or
flashing a fresh device you should upload the filesystem once:

```bash
/Users/wangkai/.platformio/penv/bin/platformio run -e m5stack-fire -t uploadfs
```

## Web Config

After flashing the firmware, you can configure the clock from Chrome over USB or
Bluetooth without entering the on-device settings flow.

Start a local web server:

```bash
./tools/start_web_config.sh
```

Then open:

```text
http://localhost:4173
```

Use Chrome or Edge. For USB, plug the device in and click `连接 USB` once to
allow the serial port; the page will reconnect to authorized USB devices on
load. For Bluetooth, click `连接蓝牙` and choose `M5Paper Clock`. The page can:

- read device status
- scan nearby Wi-Fi networks
- save SSID and password
- save timezone and clock style
- connect Wi-Fi and sync time
- trigger a full refresh or reboot

USB and Bluetooth share the same JSON command protocol. Only lines prefixed with
`@cfg:` are treated as protocol messages, so normal device logs can continue to
print on the USB serial port.

## Update Holiday Assets

Refresh the embedded holiday table manually when you want newer holiday data:

```bash
node tools/update_holiday_data.mjs 2026 2027
```

## Test

```bash
/Users/wangkai/.platformio/penv/bin/platformio test -e native
```

## Upload

Replace the upload port with the one shown by `platformio device list`.

One command for both the SPIFFS font and the firmware:

```bash
./tools/flash_all.sh /dev/cu.usbserial-02120D19
```

If you omit the port, the script will try to auto-detect a common USB serial
device.

```bash
env PLATFORMIO_CORE_DIR=/Users/wangkai/Developer/github/m5-paper-clock/.pio-core \
  /Users/wangkai/.platformio/penv/bin/platformio run -e m5stack-fire -t upload \
  --upload-port /dev/cu.usbserial-02120D19
```

If the device has not received the bundled font yet, upload SPIFFS first:

```bash
env PLATFORMIO_CORE_DIR=/Users/wangkai/Developer/github/m5-paper-clock/.pio-core \
  /Users/wangkai/.platformio/penv/bin/platformio run -e m5stack-fire -t uploadfs \
  --upload-port /dev/cu.usbserial-02120D19
```
