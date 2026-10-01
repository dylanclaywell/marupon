# Roadmap

Last updated: 2026-10-01. This file records where the project stands. Task tracking lives in
`docs/feature-backlog.md` (DrBacklog), not here.

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
  freed). It lives in a `Framebuffer` class in `main.cpp` that also owns the dirty rectangle (a
  `NullableRect`) and the LCD pointer. Drawing primitives are `fillRect` and `drawSprite`, both
  clipped to a region.
- **Sending it:** `Framebuffer::flush()` sends only the dirty rectangle. A rect of up to
  `STAGING_PIXELS` pixels (20,000, an arbitrary size) is copied into a packed `_staging` array and
  sent in one `drawBitmap` call; a larger rect falls back to one call per row. The earlier full-frame
  version (strips, despite the "40-row" in older notes the strip height was really 41, because
  `FB_HEIGHT / 10` is integer division) took about 31 ms (about 11 MB/s), steady, with no DMA errors.
- **Measured on the device:** a 100x100 update (the 96x96 sprite plus alignment and one pixel of
  movement) takes about 1.8 ms to flush, steady between 1.80 and 1.82 ms. That is about 11 MB/s again,
  so the cost is the amount of data on the link, not per-call overhead. This covers the memcpy and the
  send together; the time spent in `renderRegion` has not been measured.
- **One call per update matters.** The first version sent one `drawBitmap` per row. It was very slow
  and the sprite visibly sheared while moving; the single-call staging version fixed both.
- **The panel needs `x` aligned to 4.** Confirmed on the device: an unaligned `x_start` makes the
  library log `x_start(158) not aligned to 4` and the picture comes out 2 pixels off. `rectAlign4`
  snaps all four of x, y, w and h outward to multiples of 4 in `markDirty`. Whether y and height need
  it is still open (see Unverified).
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
- **Dirty-rectangle drawing works.** Moving the sprite marks its old and new bounds dirty
  (`markDirty` clamps to the screen, snaps to 4, and merges into one rectangle with `rectUnion`);
  `renderRegion()` then repairs the background and redraws the sprite inside that region, and
  `flush()` sends just that rectangle. No trail, and no warbling. The scene logic (`renderRegion`)
  stays outside the `Framebuffer` class because it knows what the scene contains. Only one dirty
  rectangle is tracked; more than one is not built because nothing needs it yet.
- **Serial output is visible while the input tool runs.** `simulate_input.py` now also prints every
  line the board sends, prefixed with `[board]`, from a reader thread. Only one program can hold the
  COM port, so the serial monitor cannot be open at the same time.

`main.cpp` clears the screen and draws the sprite once at startup. `loop()` then reads any pending
button input, nudges the sprite on UP/DOWN, and renders and sends only the changed rectangle.
Movement is one pixel per pass through `loop()`, so its speed depends on how fast the loop runs
(see the backlog).

All work is committed on `main`. Nothing has been pushed, and there is no remote yet.

## Next

What comes next, including cleanup and open design decisions (integer scaling, indexed sprites for
palette swaps, a fixed timestep), is tracked in `docs/feature-backlog.md`, not here.

## Notes

- `.vscode/` is mostly gitignored. Only `extensions.json` is tracked.

## Unverified

- **Touch values** come from Waveshare's wiki and example repo, not from our own testing: controller
  SPD2010, I2C address `0x53`, interrupt on GPIO 4, reset on expander pin EXIO1. The library counts
  expander pins from 0, so EXIO1 is probably index 0, but check it (the LCD reset on EXIO2 worked as
  index 1).
- **Whether y and height need 4-pixel alignment.** Only the `x_start` warning has been seen. The
  old full-frame strips started at `y = 41, 82, ...` and showed no problem, so probably only x and
  width matter, but that has not been tested.
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
  open `http://localhost:8000` in a browser. Arrow keys move UP/DOWN, `j`/`k` are A/B. The tool also
  prints everything the board sends back, prefixed with `[board]`, so use it instead of the serial monitor.
- Values not yet measured (time spent in `renderRegion`, PSRAM headroom under a real workload)
  should be measured on the device, not assumed.
