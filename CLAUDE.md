# marupon

Firmware for the **ESP32-S3 Touch 2.1** board: PlatformIO + Arduino framework, LVGL v8.4 UI,
display and touch through Espressif's `ESP32_Display_Panel` library. The project is young:
`src/main.cpp` is still the stock "Hello World" example, and there are no tests, CI or README yet.
Add conventions to this file as they are established, not ahead of the code.

## How we work together (read this first)

The owner is **learning** embedded and ESP32 development. Most sessions are questions, explanations
and debugging of the owner's own code, not delegated implementation.

- **Teach and advise by default.** Explain what a piece of code does and why, point at the relevant
  lines, and suggest the change. Plain English; define a term the first time it appears.
- **Never change anything without asking first.** That covers editing files, installing libraries,
  changing `platformio.ini`, and running builds that write to the tree. Propose the change, wait for a yes.
- **Debug the owner's code, don't rewrite it.** Find the cause, explain it, and offer the smallest fix.
- **Never flash the device or open the serial monitor.** No `pio run -t upload`, no
  `pio device monitor`, no `esptool`. The owner does that. Ask them to paste the serial output.
  Building (`pio run -e ...`) is fine once approved.

## The constraint that shapes everything

The code runs on real hardware that Claude cannot see or touch, so "it compiles" proves little.
Display, touch, PSRAM and timing bugs only show on the device. When debugging, ask for serial
output and describe what to look for instead of guessing.

## Build setup (things that will bite)

- **`default_envs` in `platformio.ini` is empty.** A bare `pio run` builds nothing. Always pass `-e <env>`.
- **The ESP32-S3 Touch 2.1 has no env in `platformio.ini` yet.** Only Espressif dev kits and
  `BOARD_CUSTOM` are listed. Which route the owner uses (custom board config in
  `src/esp_panel_board_custom_conf.h`, or a supported-board macro) is not recorded; ask, then
  write the answer here.
- **The platform is a third-party pioarduino build, pinned with Arduino core 3.1.1.** The official
  `espressif32` platform did not support Arduino 3.1.x when this was set up. Don't "fix" it back to the official platform.
- **`framework-arduinoespressif32-libs` is the `-h` (high performance) build** on purpose: it avoids
  the ESP32-S3 RGB LCD screen drift. Don't swap it for the default libs.
- **`LV_COLOR_16_SWAP` depends on the panel type:** `1` for SPI/QSPI LCDs, `0` for RGB/MIPI LCDs
  (`[spi_qspi_lcd]` and `[rgb_mipi_lcd]` in `platformio.ini`). Wrong values give swapped colours.
- **LVGL is not thread-safe.** Any LVGL call outside the port's own task needs
  `lvgl_port_lock(-1)` / `lvgl_port_unlock()` (see `src/main.cpp`).
- **Config headers in `src/`** (`lv_conf.h`, `esp_panel_*_conf.h`, `esp_utils_conf.h`) come from the
  library templates. Change values there deliberately and explain the change; don't reformat them.
- `.pio/` is build output and dependency cache and is gitignored. Never edit it.

## Commands

From `platformio.ini`; every one needs an env name (`<env>`), and the owner runs flashing:

```
pio run -e <env>                 # build only (Claude may run, after approval)
pio run -e <env> -t clean        # clean build output
pio run -e <env> -t upload       # flash  -- OWNER ONLY
pio device monitor -b 115200     # serial monitor  -- OWNER ONLY (monitor_speed = 115200)
```

## Working agreement

- **Committing:** You may commit, but only after explicitly asking "OK to commit?" and
  receiving explicit approval. Never commit without that back-and-forth. This applies to
  **every** commit — approval for one is not approval for the next, and small follow-up
  fixes during a debugging loop are exactly where this gets forgotten.
- **Commit messages:** Conventional Commits format. Body is one paragraph at most (omit it
  when the subject says enough). Do NOT append "Co-Authored-By: Claude" or any trailer.
- **The message describes everything in the commit.** Run `git status` and `git diff`
  first and account for all of it, not just the most recent edit.
- **Don't sweep up unrelated work.** If the tree holds changes outside what was asked
  for, name them and ask whether they belong in this commit. Staging is part of what
  you're asking approval for, so it gets stated, never assumed.
- **Iteration size:** Keep changes small and committable. Get approval, commit, keep moving.
- **Larger changes:** Plan in slices/phases and work through them together — no big-bang diffs.
- **Never push.** Leave the branch ahead of origin and say so; the owner pushes when ready. Don't ask
  for permission to push either. There is no remote yet, so nothing fires on a push today. If a push
  is ever rejected or history diverges, report it and stop; don't resolve it unasked.
- **Branching:** Commit directly to `main`; don't create branches or open PRs unless asked. This is a
  one-person repo with no history yet, so trunk-based is deliberate, not an oversight.
- **Commit format is convention only:** nothing parses commit subjects (no release tooling), but keep
  Conventional Commits consistent so adopting it later needs no rewrite.
- **Never deploy or publish.** There is no pipeline; the firmware only reaches hardware when the owner
  flashes it. Stay out of any CI configuration unless the task is explicitly about CI.
- **Strict defaults, no need to ask:** never rewrite published history, never skip hooks
  (`--no-verify`) or signing, never edit generated output (`.pio/`) instead of its source, never add a
  library where an existing one already does the job, never commit secrets (Wi-Fi credentials, API keys).
- **Match here-string syntax to the shell.** The PowerShell tool uses `@'` … `'@` with
  the terminator at column 0; the Bash tool uses a heredoc. `@'` under Bash lands a
  literal `@` in the commit subject. If the message contains double quotes, write it to
  a temp file and use `git commit -F <file>`. Verify with `git log -1 --format=%s`.
- **Prefer the PowerShell tool for shell work.** The Bash tool here lacks coreutils (`ls` etc.).

## Verification

No test framework is configured (`test/` holds only the PlatformIO placeholder). "Verified" means:
`pio run -e <env>` builds cleanly, and the owner flashes the board and confirms the behaviour on
screen or in the serial output. Claude can only vouch for the first half, so say so.

## Conventions

- Prefer the simplest viable change; iterate from real data. Don't add machinery for hypothetical cases.
- Don't assert numbers (buffer sizes, timings, pin choices) as if reasoned. Say when a value is
  arbitrary, and say how to measure the right one.
- Comments and docs in plain, full-sentence English that say what the code *is*, not what it used to be.
- Task tracking lives in `docs/feature-backlog.md` (DrBacklog), not in this file.
