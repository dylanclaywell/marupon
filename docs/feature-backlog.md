# DrBacklog

## TODO
- [ ] [#2: Give the serial input handling in loop() some structure](#task-2)
- [ ] [#3: Design the pet: states, animation timing, and what button input does](#task-3)
- [ ] [#4: Re-enable touch input (low priority)](#task-4)
- [ ] [#8: Decide on integer scaling (small framebuffer, scale on send)](#task-8)
- [ ] [#9: Indexed sprites with palettes (for pet colour variants)](#task-9)
- [ ] [#10: markDirty: return early on an empty visible rect before aligning](#task-10)
- [ ] [#11: Check whether only x and width need 4-pixel alignment](#task-11)
- [ ] [#12: Care stats: Hunger, Cleanliness and Fun with passive decay](#task-12)
- [ ] [#13: Cleanliness: extra decay per minigame or interaction](#task-13)
- [ ] [#14: Derived Health value from the three care stats](#task-14)
- [ ] [#15: Sickness and death timers driven by Health and one pause flag](#task-15)
- [ ] [#16: Randomized lifespan per pet](#task-16)
- [ ] [#17: Save pet state and reconcile elapsed time on boot](#task-17)
- [ ] [#18: Hatch bubble and dark-blob growth animation](#task-18)
- [ ] [#19: Pop the bubble by touch or the accept button](#task-19)
- [ ] [#20: Hatch roll: baby design, shape variant and weighted color](#task-20)
- [ ] [#21: Rare-color flourish (stars) at the pop](#task-21)
- [ ] [#22: Start the pet's clocks at the pop](#task-22)
- [ ] [#23: Zoomies and Thinkies bars with a per-stage budget](#task-23)
- [ ] [#24: Stage-growth indicator](#task-24)
- [ ] [#25: Stage schedule: evolve on the pet's own clock](#task-25)
- [ ] [#26: Baby to child path pick (Scholar or Athletic)](#task-26)
- [ ] [#27: Child to adult pick from the low/high grid](#task-27)
- [ ] [#28: Freeze Zoomies and Thinkies at adulthood](#task-28)
- [ ] [#29: Time-scale multiplier applied to all rates](#task-29)
- [ ] [#30: Sleep window setting and clock source](#task-30)
- [ ] [#31: Decay stops while asleep; babies and children follow the window](#task-31)
- [ ] [#32: Adult bedtime offset per form](#task-32)
- [ ] [#33: Lights-out check as a small mood nudge](#task-33)
- [ ] [#34: Feed: menu, food item, touch drag and button path](#task-34)
- [ ] [#35: Wash: visible grime, soap build-up, touch scrub and button taps](#task-35)
- [ ] [#36: Petting raises Fun](#task-36)
- [ ] [#37: Toy mode: self-play and direct interaction](#task-37)
- [ ] [#38: Per-toy cooldown and touch bonus](#task-38)
- [ ] [#39: Runner minigame (feeds Zoomies)](#task-39)
- [ ] [#40: Odd-one-out puzzle minigame (feeds Thinkies)](#task-40)
- [ ] [#41: Minigame results feed stats and Fun](#task-41)
- [ ] [#42: Coin wallet and toy shop](#task-42)
- [ ] [#43: Care-quality ratio on the pet's own clock](#task-43)
- [ ] [#44: Memorial tiers: default, wings, wings + halo](#task-44)
- [ ] [#45: Full past-pets list with favorites](#task-45)
- [ ] [#46: Snowglobe of past pets](#task-46)
- [ ] [#47: In-game bestiary](#task-47)
- [ ] [#48: Room screen with a wandering pet](#task-48)
- [ ] [#49: Menu system that hides the room](#task-49)
- [ ] [#50: Care stat display](#task-50)
- [ ] [#51: Button input layer: UP/DOWN, ACCEPT, BACK](#task-51)
- [ ] [#52: Decide pixel density using mockups on the real panel](#task-52)
- [ ] [#53: Sprite pipeline for 14 designs times colors](#task-53)
- [ ] [#54: Pixel font and text rendering](#task-54)
- [ ] [#55: Sync panel updates to the TE pin to stop tearing and beat](#task-55)

## DONE
- [x] [#1: Decouple game logic and movement from loop speed (fixed timestep)](#task-1)
- [x] [#5: Remove LVGL once nothing needs it](#task-5)
- [x] [#6: Remove the unused Espressif envs and board JSON files](#task-6)
- [x] [#7: Tidy leftovers in BOARD_CUSTOM.json and the outdated config headers](#task-7)

## CLOSED

---

## Task Details

<a id="task-1"></a>
### #1: Decouple game logic and movement from loop speed (fixed timestep)
* **Status:** DONE
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
* **Status:** DONE
* **Created:** 2026-10-01
* **Description:** main.cpp no longer uses LVGL, but it is still built in. Remove the lvgl entry in lib_deps and the LVGL flags in platformio.ini, plus src/lv_conf.h, src/lvgl_v8_port.cpp and src/lvgl_v8_port.h. Check afterwards that LV_COLOR_16_SWAP removal does not change colours (our own code already does the byte swap in panelColor()). Build with pio run -e BOARD_CUSTOM; flashing and confirming the picture is the owner's step.
* **Resolution:** Removed LVGL (lib_deps, flags, lv_conf.h, lvgl_v8_port.*) in d802842. Builds; owner flashed and saw no change.

<a id="task-6"></a>
### #6: Remove the unused Espressif envs and board JSON files
* **Status:** DONE
* **Created:** 2026-10-01
* **Description:** platformio.ini still has the Espressif envs and boards/ still has their JSON files. Building any of them for this board fails with "Multiple boards enabled" (see CLAUDE.md), so they only cause confusion. Remove them and keep BOARD_CUSTOM. Update the Build setup notes in CLAUDE.md afterwards if they mention the other envs.
* **Resolution:** Removed the Espressif envs and board JSON files in d802842; BOARD_CUSTOM is the only env. CLAUDE.md build notes updated.

<a id="task-7"></a>
### #7: Tidy leftovers in BOARD_CUSTOM.json and the outdated config headers
* **Status:** DONE
* **Created:** 2026-10-01
* **Description:** boards/BOARD_CUSTOM.json was copied from an Espressif board; its name and url fields are leftovers and do not affect the build. The build also prints "file version is outdated" warnings for src/esp_panel_drivers_conf.h and src/esp_panel_board_supported_conf.h. They are harmless; the fix is to refresh those two files from the library's current templates, changing values deliberately and explaining any change, and keeping the supported-board flag at 0.
* **Resolution:** Refreshed esp_panel_drivers_conf.h and esp_panel_board_supported_conf.h from the library templates (no values changed, supported-board flag still 0) and set BOARD_CUSTOM.json name, url and vendor for this board, in 045c9f7. Owner built and uploaded fine.

<a id="task-8"></a>
### #8: Decide on integer scaling (small framebuffer, scale on send)
* **Status:** TODO
* **Created:** 2026-10-01
* **Description:** For a retro look, the idea is to draw into a small logical framebuffer and scale up. 412 divides evenly: 4x gives 103x103 (about 21 KB, possibly small enough for internal RAM), 2x gives 206x206. Scaling at send means flush() expands each logical row into a physical row (each pixel repeated S times) and sends it; the dirty rect is scaled by S, and with S = 4 it is automatically aligned to 4. Staging size depends on S. Alternative: keep 412x412 and scale at blit time (simpler, keeps the big buffer). Decide before polishing the flush further, because it changes what the framebuffer is. Suggested approach: keep the expansion loop in flush() working at S = 1 first, then raise S and time both (draw, expand, send; sprite-sized rect and full screen) with micros(). Nothing is measured yet.
* **Progress (2026-10-01):** The mechanism is built and works at 1x, 2x and 4x: a canvas/panel split with a SCALE constant in main.cpp, flush() expanding the dirty canvas rect into the staging buffer and sending it in bands, and alignment that follows SCALE (4 / SCALE canvas pixels). SCALE is still 1. With the 96x96 baby sprite at 4x the loop dropped to about 47 ticks/s and showed some tearing, since that sprite fills almost the whole screen. Still to do: test with the new 32x32 art (convert the PNG first), time the draw, expand and send stages with micros() for a sprite-sized rect and the full screen, then choose the scale and record the numbers.

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

<a id="task-12"></a>
### #12: Care stats: Hunger, Cleanliness and Fun with passive decay
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** Hunger and Fun decay steadily with time; Cleanliness drifts down slowly on its own. Decay rates are unset in the design; make them constants that are easy to change and tune by feel. Notion: Stats & Care.

<a id="task-13"></a>
### #13: Cleanliness: extra decay per minigame or interaction
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** Each minigame or interaction costs a flat Cleanliness hit, tuned as a mild nuisance (a wash every several games). Needs the minigames (E6) to exist before it can be tuned.

<a id="task-14"></a>
### #14: Derived Health value from the three care stats
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** Health is recalculated continuously from Hunger (highest weight), Cleanliness, then Fun. It has no decay of its own and recovers immediately when the stats recover. The weights are an open question. It is hidden from the player and shown only through symptoms.

<a id="task-15"></a>
### #15: Sickness and death timers driven by Health and one pause flag
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** At Health zero, 5 minutes of active time before sickness, then 15 more before death. Both timers check a single pause flag, regardless of why it is set. Death is possible from the first pet. The windows are intentionally large and may be tuned.

<a id="task-16"></a>
### #16: Randomized lifespan per pet
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** Each pet rolls its own lifespan so there is no fixed 'dies at day X'. Measured on the pet's own clock (see E4). The range is not decided.

<a id="task-17"></a>
### #17: Save pet state and reconcile elapsed time on boot
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #1
* **Description:** Store the pet and a last-saved timestamp, and on boot compare against the RTC. A power loss is treated exactly like neglect, with no special case. The storage choice (NVS, flash file) is open. The Notion docs list the RTC as onboard; check the 1.46 board has one.

<a id="task-18"></a>
### #18: Hatch bubble and dark-blob growth animation
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #2
* **Description:** A bubble floats in and bobs, and a dark blob grows inside it. The blob must be identical every time (shape, size, darkness, animation) so nothing about the pet leaks. Nothing else in the game runs while it waits.

<a id="task-19"></a>
### #19: Pop the bubble by touch or the accept button
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #2
* **Description:** Both inputs give the same outcome. The pet pops out and reveals itself. Open questions: how long the blob grows before it can be popped, and whether popping early changes anything.

<a id="task-20"></a>
### #20: Hatch roll: baby design, shape variant and weighted color
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #2
* **Description:** Each hatch is an independent roll (no inheritance). Shape is baby-only and cosmetic; color is cosmetic and carries into adulthood. One weighted common/rare tier for now. Pool sizes and odds are undecided.

<a id="task-21"></a>
### #21: Rare-color flourish (stars) at the pop
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #2
* **Description:** Only at the pop, so the blob still gives nothing away. A single rare tier means a single effect.

<a id="task-22"></a>
### #22: Start the pet's clocks at the pop
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #2
* **Description:** No stats, aging, lifespan, care-quality clock or sickness timers exist until the pop, so the bubble needs no pause. Applies to every hatch, including new babies after a death or a grown-up pet.

<a id="task-23"></a>
### #23: Zoomies and Thinkies bars with a per-stage budget
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** Two separate bars that only go up and are fed only by minigames. Each stage has a shared budget (pure tradeoff, no moving points back), and bars reset at each stage. Once the budget is spent, minigames still give Fun but no longer move the bars. Budget and threshold values are open.

<a id="task-24"></a>
### #24: Stage-growth indicator
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** Shows how much of the stage's budget is spent, and gives the 'growth is done for now' state a visible home. Not needed for adults. Layout on the round screen is open.

<a id="task-25"></a>
### #25: Stage schedule: evolve on the pet's own clock
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** Baby, child and adult stages change by age, not when the budget fills. Playing never speeds growth. Stage lengths (the baby stage is shortest) are undecided. Scaled by slow-down (E4).

<a id="task-26"></a>
### #26: Baby to child path pick (Scholar or Athletic)
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** Compare Zoomies and Thinkies at the end of the baby stage; the higher one picks the path (Thinkies higher gives Scholar, Zoomies higher gives Athletic). A tie, including a baby that never played, picks randomly. The child design within a path is a random cosmetic pick.

<a id="task-27"></a>
### #27: Child to adult pick from the low/high grid
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** The bars have reset, so only the child stage's play counts. Each stat is low or high at the middle threshold, giving four adults per path (8 in total). The budget must exceed the threshold, and be at least twice it for 'both high'.

<a id="task-28"></a>
### #28: Freeze Zoomies and Thinkies at adulthood
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #3
* **Description:** The final values stay on show as a record of how the pet was raised. They stop changing.

<a id="task-29"></a>
### #29: Time-scale multiplier applied to all rates
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #4
* **Description:** One multiplier stretches decay, growth, aging and toy cooldowns together. Anchors (sleep window, lights-out) stay on the real clock. Open: whether it can change mid-life or is fixed at hatch, tied to the player-chosen difficulty.

<a id="task-30"></a>
### #30: Sleep window setting and clock source
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #4
* **Description:** The player sets a 12-hour window (for example 9PM to 9AM). Needs a clock source (onboard RTC) and a plan for time zones and daylight saving. Check which RTC the 1.46 board has.

<a id="task-31"></a>
### #31: Decay stops while asleep; babies and children follow the window
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #4
* **Description:** Before adulthood the pet sleeps and wakes exactly on the window. Open: what happens if the player interacts mid-sleep.

<a id="task-32"></a>
### #32: Adult bedtime offset per form
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #4
* **Description:** Each adult form has a fixed bedtime within plus or minus 1 hour of the window, stored as one design-data entry per form (8 in v1). The lights-out check happens at the pet's own bedtime.

<a id="task-33"></a>
### #33: Lights-out check as a small mood nudge
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #4
* **Description:** Affects only happiness and fun, never health or death. Not a care check.

<a id="task-34"></a>
### #34: Feed: menu, food item, touch drag and button path
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #5
* **Description:** Both paths play the same eating animation (button feed is not instant). Touch drags the food to the pet; button presses to feed. The same outcome either way. Needs the menu (E8).

<a id="task-35"></a>
### #35: Wash: visible grime, soap build-up, touch scrub and button taps
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #5
* **Description:** Dirt shows on the pet. Touch scrubbing builds up soap until a coverage threshold, then a clean animation plays. The button path is the equivalent number of taps. The scratch-off reveal was rejected.

<a id="task-36"></a>
### #36: Petting raises Fun
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #5
* **Description:** A touch action. No button equivalent is defined yet, and limits or cooldown are open. Touch is currently disabled (see #4).

<a id="task-37"></a>
### #37: Toy mode: self-play and direct interaction
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #5
* **Description:** Toy mode keeps the room visible. The pet self-plays for a limited time, and direct play (for example tossing a ball) raises Fun more. Outside toy mode the room is not touch-interactable. How the mode is entered and exited is open.

<a id="task-38"></a>
### #38: Per-toy cooldown and touch bonus
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #5
* **Description:** After a session a toy cannot raise Fun until its cooldown ends (scaled by slow-down). Button-only play must still be enough to keep Fun healthy; touch only adds a bonus. Cooldown and bonus sizes are open.

<a id="task-39"></a>
### #39: Runner minigame (feeds Zoomies)
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #6
* **Description:** Time-based jump-over-obstacles game; score is distance covered when time runs out. Time limit and in-run difficulty ramp are open.

<a id="task-40"></a>
### #40: Odd-one-out puzzle minigame (feeds Thinkies)
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #6
* **Description:** Three pet-themed cards, pick the odd one. Difficulty follows a fixed ladder: different images, then colors, then positions, then rotation. Time-based; score is how far up the ladder you get.

<a id="task-41"></a>
### #41: Minigame results feed stats and Fun
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #6
* **Description:** On finish: update Zoomies or Thinkies within the stage budget, raise Fun, apply the Cleanliness hit and pay coins. A flat hit is used because both games are time-bounded.

<a id="task-42"></a>
### #42: Coin wallet and toy shop
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #6
* **Description:** Coins scale with performance and keep paying after the growth budget is spent. They buy more toys, and the player starts with at least one. Toy list, prices and payout formula are open.

<a id="task-43"></a>
### #43: Care-quality ratio on the pet's own clock
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #7
* **Description:** Care given against care needed, relative to the pet's own lifespan, used only for the memorial tier. The formula is the main open question (whether it reflects stats kept out of the red). Needs the stats (E1) and the time scale (E4).

<a id="task-44"></a>
### #44: Memorial tiers: default, wings, wings + halo
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #7
* **Description:** Tiers only add embellishment. Default is most pets, wings is solid care, wings + halo is real dedication. Needs sprite overlays. More tiers than three are an open question.

<a id="task-45"></a>
### #45: Full past-pets list with favorites
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #7
* **Description:** Every past pet with its sprite, failures included, is a permanent record. The player pins favorites here (1 to 3). Layout on the round screen is open.

<a id="task-46"></a>
### #46: Snowglobe of past pets
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #7
* **Description:** A non-interactive ghostly scene of up to 8 pets (1 to 3 pinned, the rest random, reshuffled on each open). Each appears at the stage it died at, with its memorial tier. The cap depends on measured performance.

<a id="task-47"></a>
### #47: In-game bestiary
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #7
* **Description:** Family page, stage page, then colors. Silhouettes until found, with rare color slots shown as '?'. An entry unlocks when a pet reaches that stage and color. Works as a completion tracker.

<a id="task-48"></a>
### #48: Room screen with a wandering pet
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #8
* **Description:** The main screen is a room containing a medium-sized pet that wanders. Furniture is not touch-interactable. Builds on the movement work in #1 and the pet design in #3.

<a id="task-49"></a>
### #49: Menu system that hides the room
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #8
* **Description:** Opening the menu hides the room until an action is chosen or the menu is closed. Needs menu structure and navigation for all of the care actions.

<a id="task-50"></a>
### #50: Care stat display
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #8
* **Description:** Show Hunger, Cleanliness and Fun (hearts, bars or something else is undecided). Health stays hidden and appears only as symptoms such as sluggish animation and sad expressions.

<a id="task-51"></a>
### #51: Button input layer: UP/DOWN, ACCEPT, BACK
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #8
* **Description:** Four front buttons: an angled previous/next pair, a bigger accept and a back. The serial simulator currently sends A/B/UP/DOWN; decide the mapping and keep serial as the stand-in until real buttons exist. Related to #2 and #3.

<a id="task-52"></a>
### #52: Decide pixel density using mockups on the real panel
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #9
* **Description:** Compare the same scene (pet in room, a menu, some text) at full density and at integer scales, viewed on the device at holding distance. The Notion docs assume 480x480; this board is 412x412 (412 divides by 2 and 4), so the candidates differ. Closely related to #8.

<a id="task-53"></a>
### #53: Sprite pipeline for 14 designs times colors
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #9
* **Description:** Every color needs a version of all 14 designs. Builds on indexed sprites and palettes (#9) so colors are palette swaps, not duplicated art. Extend the PNG converter in tools/.

<a id="task-54"></a>
### #54: Pixel font and text rendering
* **Status:** TODO
* **Created:** 2026-10-01
* **Epic:** #9
* **Description:** Once LVGL is gone (#5) all text is ours. Readable text needs a minimum number of art pixels per letter and must share the one pixel density used by the rest of the screen.
---

<a id="task-55"></a>
### #55: Sync panel updates to the TE pin to stop tearing and beat
* **Status:** TODO
* **Created:** 2026-10-02
* **Epic:** #9
* **Description:** Moving the sprite showed row-by-row jumps in slow motion. Cause: the panel rescans its own copy of the picture at about 61 Hz (measured: the TE signal on GPIO 18 pulses about 61 times per second) while our writes landed at arbitrary moments, so a seam appeared; our 16 ms tick (about 62.5 Hz) also beats against the 61 Hz refresh. The library already turns TE on (default SPD2010 init table, 0x35 with 0x00) and does nothing with the pin. Owner writes the code. Slice 3a is DONE (ee56cf0): flush() waits for a TE pulse after expanding the first band and before sending it (25 ms timeout, first band only), so a sprite-sized update is sent right after the pulse. Confirmed on the device: no row-by-row jumps, te/s 61, 0 wait timeouts, longest drawBitmap about 2.7 to 2.9 ms and longest expansion about 0.24 to 0.26 ms (against a scan of about 16.4 ms), so one sprite update uses roughly 18% of a refresh and the bus send, not the CPU expansion, is the expensive part. Slice 3b is TODO: run process() once per TE pulse instead of every 16 ms, falling back to the timer if pulses stop, to remove the beat (a hop about 1.5 times a second). Still unmeasured: the time renderRegion takes and whether the rising or falling TE edge is the better one (not needed while the picture looks right). Known limit: a full-frame write (about 31 ms measured earlier) is longer than one scan and can still tear. Related to #8.

## Epics

<a id="epic-1"></a>
### Epic #1: Care stats & health
* **Description:** Hunger, Cleanliness and Fun, the derived Health value, sickness and death, lifespan, and saving state. Design source: Marupon hub in Notion (E1).

<a id="epic-2"></a>
### Epic #2: Hatching
* **Description:** The hatch bubble, the pop, the hatch roll and starting the pet's clocks. Design source: Marupon hub in Notion (E2).

<a id="epic-3"></a>
### Epic #3: Growth & evolution
* **Description:** Zoomies and Thinkies bars, stage schedule and indicator, and the baby to child to adult picks. Design source: Marupon hub in Notion (E3).

<a id="epic-4"></a>
### Epic #4: Sleep & slow-down
* **Description:** Time-scale multiplier, sleep window, decay while asleep, bedtime offsets and lights-out. Design source: Marupon hub in Notion (E4).

<a id="epic-5"></a>
### Epic #5: Care interactions
* **Description:** Feeding, washing, petting and toys. Design source: Marupon hub in Notion (E5).

<a id="epic-6"></a>
### Epic #6: Minigames & coins
* **Description:** The Runner and odd-one-out minigames, how results feed stats, and the coin wallet and toy shop. Design source: Marupon hub in Notion (E6).

<a id="epic-7"></a>
### Epic #7: Memorial & history
* **Description:** Care-quality ratio, memorial tiers, past-pets list, snowglobe and bestiary. Design source: Marupon hub in Notion (E7).

<a id="epic-8"></a>
### Epic #8: UI & room
* **Description:** The room screen, menu system, care stat display and button input layer. Design source: Marupon hub in Notion (E8).

<a id="epic-9"></a>
### Epic #9: Art & rendering
* **Description:** Pixel density, the sprite pipeline, and pixel font and text rendering. Design source: Marupon hub in Notion (E9).
