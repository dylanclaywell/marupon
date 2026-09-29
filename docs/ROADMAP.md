# Roadmap

Last updated: 2026-09-28. This file records where the project stands and what comes next. Task
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
- **Sprite blit and transparency work.** `drawSprite()` copies a `const uint16_t` sprite array into
  the framebuffer, skipping pixels equal to the magenta colour key `0x1FF8`. Verified visually on the
  device with `sprite_baby.h`.
- **Simulated input works end-to-end, with no physical buttons wired up.** `tools/simulate_input/` is
  a local Python tool (an `http.server` app plus a small HTML/JS page) that lets UP/DOWN/A/B "buttons"
  be pressed from the laptop keyboard or on-screen and forwarded over USB serial as text lines
  (`BUTTON <NAME> PRESSED`/`BUTTON <NAME> RELEASED`). `main.cpp`'s `loop()` reads `Serial`
  non-blockingly into a fixed `char` line buffer, parses completed lines with `sscanf`, and updates a
  `buttonStates[]` array. UP/DOWN currently just nudge the sprite's Y position by one pixel per frame
  as a smoke test — no real pet behaviour is designed yet.
- **The sprite move-test doesn't erase behind itself yet.** Each `loop()` call draws the sprite at its
  current `spriteX`/`spriteY` without clearing the previous position first, so holding UP/DOWN
  currently leaves a trail on screen. Expected: dirty-rectangle drawing (see Next) hasn't been built.

`main.cpp` currently draws the test pattern once, then repeatedly sends the whole frame, reads any
pending button input, and nudges the sprite. That loop is a measurement and a smoke test, not the
final design.

All work is committed on `main`. Nothing has been pushed, and there is no remote yet.

## Next

1. **Clean up and organize the input-handling code in `main.cpp`.** The line-buffer reading, `sscanf`
   parsing and `buttonStates[]` updates in `loop()` were built incrementally and work, but landed as
   one long block; give them structure before building more on top.
2. **Dirty-rectangle drawing.** Decide how sprite movement tracks and redraws only the changed area,
   instead of leaving a trail behind the sprite every frame. This folds in the old `flushRect(x, y, w,
   h)` idea: measure how long a small rectangle takes to send compared with the 31 ms full frame, and
   keep x, y, width and height multiples of 4 until we know whether the panel needs that.
3. **Startup clear.** Push one full clear frame at boot, because the panel keeps its old picture
   across reboots.
4. **The pet itself:** states, animation timing, and what button input does. Nothing beyond nudging
   the sprite up/down as a smoke test is designed yet.
5. **Touch.** Still switched off (`ESP_PANEL_BOARD_USE_TOUCH (0)`) and now lower priority: the current
   input model is the named buttons (UP/DOWN/A/B) above, simulated from the laptop over serial rather
   than real touch coordinates. See the unverified values below if touch gets picked back up later.
6. **Remove LVGL** once nothing needs it: `lvgl` in `lib_deps`, the LVGL flags in `platformio.ini`,
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
- Simulated input (no physical buttons needed): `uv run tools/simulate_input/simulate_input.py`, then
  open `http://localhost:8000` in a browser. Arrow keys move UP/DOWN, `j`/`k` are A/B.
- Values not yet measured (frame times with partial rectangles, PSRAM headroom under a real workload)
  should be measured on the device, not assumed.
