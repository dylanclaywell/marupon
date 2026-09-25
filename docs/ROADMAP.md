# Roadmap

Last updated: 2026-09-25. This file records where the project stands and what comes next. Task
tracking proper lives in `docs/feature-backlog.md` (DrBacklog), which is still empty; the "Next"
list below is the seed for it.

## Goal

A virtual pet on the Waveshare ESP32-S3-Touch-LCD-1.46 (412x412 round display, SPD2010 over QSPI,
16 MB flash, 8 MB PSRAM). The pet is drawn by our own code into a framebuffer in PSRAM (sprites,
blitting) and pushed to the panel. LVGL was only used as a temporary smoke test and is planned to
be removed.

## Where we left off

Everything below was confirmed on the device by the owner, unless marked otherwise.

- **Board bring-up works.** The board has no preset in `ESP32_Display_Panel` 1.0.5, so it uses a
  hand-written `src/esp_panel_board_custom_conf.h` and the `BOARD_CUSTOM` env. The LCD reset goes
  through the TCA9554 IO expander in a pre-begin hook.
- **Drawing without LVGL works.** `main.cpp` no longer uses LVGL. It draws with
  `LCD::drawBitmap()` directly.
- **Pixel format:** RGB565 with the two bytes of each pixel swapped. `panelColor(r, g, b)` in
  `main.cpp` is the only place that swap happens.
- **Framebuffer:** 412x412x2 = 339,488 bytes, allocated once in PSRAM at startup (`ps_malloc`, never
  freed). The primitives so far are `setPixel` and `fillRect`.
- **Sending it:** `flushFramebuffer()` sends the whole buffer in 40-row strips. A full frame takes
  about 31 ms (about 11 MB/s), steady, with no DMA errors.
- **The test pattern renders correctly:** dark blue background, white outline, and markers red
  top-left, green top-right, blue bottom-left, yellow bottom-right. So the panel's axes match the
  buffer and no mirror or swap flags are needed.
- **Serial works** over USB (`ARDUINO_USB_CDC_ON_BOOT=1` in `boards/BOARD_CUSTOM.json`). The port
  resets on every reboot, so `setup()` waits 5 s before printing.

`main.cpp` currently draws the test pattern once, then re-sends the whole frame every second and
prints how long it took. That loop is a measurement, not the final design.

All work is committed on `main`. Nothing has been pushed, and there is no remote yet.

## Next

1. **Sprite blit.** Make a tiny hand-written sprite as a `const uint16_t` array (that is how exported
   sprites will arrive), copy it into the framebuffer with our own loop, and check that it appears
   where expected. Decide whether sprite data is stored already byte-swapped or swapped when blitting.
2. **Transparency.** Pick a "transparent" colour key, skip those pixels in the blit, and test it over
   the test pattern's background.
3. **Send only the changed rectangle** (`flushRect(x, y, w, h)`), and measure how long a small
   rectangle takes compared with the 31 ms full frame. Keep x, y, width and height multiples of 4
   until we know whether the panel needs that.
4. **Startup clear.** Push one full clear frame at boot, because the panel keeps its old picture
   across reboots.
5. **Touch.** Currently switched off (`ESP_PANEL_BOARD_USE_TOUCH (0)`). See the unverified values
   below.
6. **The pet itself:** states, animation timing, and what touch does. Nothing is designed yet.
7. **Remove LVGL** once nothing needs it: `lvgl` in `lib_deps`, the LVGL flags in `platformio.ini`,
   `src/lv_conf.h`, `src/lvgl_v8_port.cpp` and `src/lvgl_v8_port.h`.

## Cleanup, when convenient

- `platformio.ini` still has the unused Espressif envs, and `boards/` still has their JSON files.
  Building any of them for this board fails with "Multiple boards enabled" (see `CLAUDE.md`).
- `boards/BOARD_CUSTOM.json` was copied from an Espressif board. Its `name` and `url` fields are
  leftovers and do not affect the build.
- The build prints "file version is outdated" warnings for `src/esp_panel_drivers_conf.h` and
  `src/esp_panel_board_supported_conf.h`. They are harmless; the fix is to refresh those files from
  the library's current templates.
- `SQ`, `square` and `drawSquare` in `main.cpp` are left over from the byte-order experiment and
  are only used for the startup cyan square.
- `.vscode/` is mostly gitignored. Only `extensions.json` is tracked.

## Unverified

- **Touch values** come from Waveshare's wiki and example repo, not from our own testing: controller
  SPD2010, I2C address `0x53`, interrupt on GPIO 4, reset on expander pin EXIO1. The library counts
  expander pins from 0, so EXIO1 is probably index 0, but check it (the LCD reset on EXIO2 worked as
  index 1).
- **Whether the panel needs 4-pixel alignment** for partial updates. The ESPHome notes suggest it;
  all our transfers so far were multiples of 4.
- **About 451 KiB of PSRAM is already taken** before our allocation (the buffer landed at
  `0x3C070DC8`). We don't know what uses it. Not a problem now (about 7.5 MB free).
- **QSPI SPI mode `0`** and **backlight on GPIO 5** work, but the mode-`0` choice was a library
  default and a wiki hint suggested mode 3. If the picture ever tears, try that.

## Resuming

- Build: `pio run -e BOARD_CUSTOM`. It only compiles. The owner flashes the board and runs the serial
  monitor.
- Read `CLAUDE.md` first. Key rules: propose changes and wait for a yes, never flash or open the
  monitor, ask "OK to commit?" for every commit, never push.
- Values not yet measured (frame times with partial rectangles, PSRAM headroom under a real workload)
  should be measured on the device, not assumed.
