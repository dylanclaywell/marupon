# DrBacklog

## TODO
- [ ] [#1: Decouple game logic and movement from loop speed (fixed timestep)](#task-1)
- [ ] [#2: Give the serial input handling in loop() some structure](#task-2)
- [ ] [#3: Design the pet: states, animation timing, and what button input does](#task-3)
- [ ] [#4: Re-enable touch input (low priority)](#task-4)
- [ ] [#5: Remove LVGL once nothing needs it](#task-5)
- [ ] [#6: Remove the unused Espressif envs and board JSON files](#task-6)
- [ ] [#7: Tidy leftovers in BOARD_CUSTOM.json and the outdated config headers](#task-7)
- [ ] [#8: Decide on integer scaling (small framebuffer, scale on send)](#task-8)
- [ ] [#9: Indexed sprites with palettes (for pet colour variants)](#task-9)
- [ ] [#10: markDirty: return early on an empty visible rect before aligning](#task-10)
- [ ] [#11: Check whether only x and width need 4-pixel alignment](#task-11)

## DONE

## CLOSED

---

## Task Details

<a id="task-1"></a>
### #1: Decouple game logic and movement from loop speed (fixed timestep)
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** Sprite movement is currently one pixel per pass through loop(), so its speed depends on how fast the loop runs; the flush optimisation made the sprite visibly faster. Run the game logic on a fixed tick instead (for example a millis() check every TICK_MS; the interval is arbitrary, tune it by feel) so speed no longer changes when the code gets faster or slower. Keep serial input reading on every pass and keep rendering "only when dirty"; use a non-blocking time check, not delay(), so input is not stalled. Decide whether missed ticks catch up (lastTick += TICK_MS) or are dropped (lastTick = now). Animation frames can count ticks rather than use a separate timer. Delta-time movement (speed * dt, needs sub-pixel positions) is the alternative if smoother motion is ever needed. Touches the input and movement code in main.cpp, so plan it as its own slice.

<a id="task-2"></a>
### #2: Give the serial input handling in loop() some structure
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** The line-buffer reading, sscanf parsing and buttonStates[] updates in loop() grew incrementally and still sit inline in loop() as one long block. Pull them into named functions (for example one that reads available bytes into the line buffer, one that handles a completed line) before building more on top. Behaviour should not change: the simulate_input tool sends BUTTON <NAME> PRESSED/RELEASED lines and the board should react exactly as now.

<a id="task-3"></a>
### #3: Design the pet: states, animation timing, and what button input does
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** Nothing beyond nudging the sprite up and down with UP/DOWN as a smoke test is designed yet. Decide the pet's states, how animation frames are timed (counting ticks once the fixed timestep exists), and what A/B/UP/DOWN mean. Probably needs to be broken into smaller tasks once the design is clear; consider a Socratic design session first.

<a id="task-4"></a>
### #4: Re-enable touch input (low priority)
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** Touch is switched off (ESP_PANEL_BOARD_USE_TOUCH is 0 in src/esp_panel_board_custom_conf.h). It is lower priority now that input comes from named buttons simulated over serial. If picked up, the values still need verifying on the device: controller SPD2010, I2C address 0x53, interrupt on GPIO 4, reset on expander pin EXIO1 (probably index 0, since the LCD reset on EXIO2 worked as index 1). See the Unverified section of docs/ROADMAP.md.

<a id="task-5"></a>
### #5: Remove LVGL once nothing needs it
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** main.cpp no longer uses LVGL, but it is still built in. Remove the lvgl entry in lib_deps and the LVGL flags in platformio.ini, plus src/lv_conf.h, src/lvgl_v8_port.cpp and src/lvgl_v8_port.h. Check afterwards that LV_COLOR_16_SWAP removal does not change colours (our own code already does the byte swap in panelColor()). Build with pio run -e BOARD_CUSTOM; flashing and confirming the picture is the owner's step.

<a id="task-6"></a>
### #6: Remove the unused Espressif envs and board JSON files
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** platformio.ini still has the Espressif envs and boards/ still has their JSON files. Building any of them for this board fails with "Multiple boards enabled" (see CLAUDE.md), so they only cause confusion. Remove them and keep BOARD_CUSTOM. Update the Build setup notes in CLAUDE.md afterwards if they mention the other envs.

<a id="task-7"></a>
### #7: Tidy leftovers in BOARD_CUSTOM.json and the outdated config headers
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** boards/BOARD_CUSTOM.json was copied from an Espressif board; its name and url fields are leftovers and do not affect the build. The build also prints "file version is outdated" warnings for src/esp_panel_drivers_conf.h and src/esp_panel_board_supported_conf.h. They are harmless; the fix is to refresh those two files from the library's current templates, changing values deliberately and explaining any change, and keeping the supported-board flag at 0.

<a id="task-8"></a>
### #8: Decide on integer scaling (small framebuffer, scale on send)
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** For a retro look, the idea is to draw into a small logical framebuffer and scale up. 412 divides evenly: 4x gives 103x103 (about 21 KB, possibly small enough for internal RAM), 2x gives 206x206. Scaling at send means flush() expands each logical row into a physical row (each pixel repeated S times) and sends it; the dirty rect is scaled by S, and with S = 4 it is automatically aligned to 4. Staging size depends on S. Alternative: keep 412x412 and scale at blit time (simpler, keeps the big buffer). Decide before polishing the flush further, because it changes what the framebuffer is. Suggested approach: keep the expansion loop in flush() working at S = 1 first, then raise S and time both (draw, expand, send; sprite-sized rect and full screen) with micros(). Nothing is measured yet.

<a id="task-9"></a>
### #9: Indexed sprites with palettes (for pet colour variants)
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** To get different-coloured pets without duplicating art, store sprites as small indices (4 or 8 bit) plus a palette of RGB565 values, and look up palette[index] in drawSprite. Transparency becomes index 0 instead of the magenta colour key. The framebuffer stays 16-bit, so how pixels reach the panel does not change. This needs the PNG-to-byte-array converter in tools/ to emit indices and a palette, and it also shrinks sprite data in flash. This is a feature, not a performance task: changing the framebuffer's own bit depth was judged premature.

<a id="task-10"></a>
### #10: markDirty: return early on an empty visible rect before aligning
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** In Framebuffer::markDirty the clamped rect is passed to rectAlign4 without checking isEmpty() first. It works today only because the rounding happens to keep off-screen rects empty, but rounding the left edge down and the right edge up can turn a zero-width rect into a 4-wide one. Add: if (visible.isEmpty()) return; before the align and union.

<a id="task-11"></a>
### #11: Check whether only x and width need 4-pixel alignment
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** The panel library warns "x_start(158) not aligned to 4" for an unaligned x, so x alignment is confirmed. The old full-frame strips were 41 rows tall (FB_HEIGHT / 10 is integer division), started at y = 41, 82, ..., and showed no problem, which suggests y and height may not need aligning. Test by snapping only x and w in rectAlign4 and leaving y and h exact: watch the serial output for warnings and the screen for shifted rows. If it works it sends slightly fewer pixels per update, and the result goes in the Unverified section of docs/ROADMAP.md as a verified fact.
