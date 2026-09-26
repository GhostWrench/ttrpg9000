# TTRPG-9000 Firmware

Open-source firmware for the TTRPG-9000, an ATtiny4313 dice-rolling
handheld. This directory (`code/`) holds the C firmware, the host test
suite, and the build/flash toolchain. Hardware design lives in the sibling
`pcb/` (KiCad) and `case/` (FreeCAD) trees — not code.

## Toolchain & build
- Cross-compiled C (C99) with `avr-gcc`; no host-only framework. All
  `make`/`container.sh` commands run from this `code/` directory.
- Build target is ATtiny4313 (`MCU`), flashed via USBasp (`ISP`) at 19200
  baud — both overridable on the command line.
- `F_CPU` is 1 MHz (see `config.h`); `main.c` is the entrypoint.
- Two firmware variants: `make` = Standard, `SHADOWRUN=1 make` = Shadowrun.
  Build dirs are tagged `build/<MCU>-<variant>/` so switching `SHADOWRUN=`
  or `MCU=` starts a fresh object tree (no stale objects).

## Build without a host toolchain (Podman)
`./container.sh` builds/flashes inside a container (source bind-mounted at
`/src`); no AVR toolchain needed on the host. Image is built on first run.
- `./container.sh build|shadowrun|docs|clean|shell` (extra args forward to
  `make`, e.g. `./container.sh build MCU=attiny85314`).
- `flash` / `shadowrun-flash` / `read-fuses` / `write-fuses` run
  `--privileged` so `avrdude` can reach the USB AVR programmer via libusb.
- Full command table in `PODMAN.md`.

## Direct make targets
- `make` / `SHADOWRUN=1 make` — build firmware → `build/<MCU>-<variant>/ttrpg9000_vMAJOR.MINOR(.hex)`.
- `make docs` — Doxygen (`Doxyfile`) → `docs/html/index.html`.
- `make flash` / `read-fuses` / `write-fuses` — needs a 3.3V AVR programmer
  on the 6-pin ISP header. Fuse bytes: lfuse 0x64, hfuse 0xDF, efuse 0xFF.

## Host unit tests (`code/test/`)
- Built with host `gcc` via `-DHOST_BUILD`; build with `make` and run with
  `make run` (inside `code/test/`).
- Tests compile host `gcc` builds of `src/*.c` directly. To keep those
  files host-compilable, AVR-only code (avr/io.h, timer/PRNG internals) is
  wrapped in `#ifndef HOST_BUILD` — mirror this in any new module compiled
  into a host test.

## Code conventions (match existing code)
- 4-space indent, Allman braces, `-Wall -Wextra -Wpedantic`.
- Module = one `src/<name>.c` + `include/<name>.h`, with matching
  Doxygen `@file/@defgroup` group headers.
- Peripherals are defined as `<NAME>_DDR/_PORT/_PIN/_BIT` macro sets in
  `config.h`, consumed by the `util.h` macros (`OUTPUT_PIN`, `INPUT_PIN`,
  `SET_PIN`, `CLR_PIN`, `READ_PIN`). Add new pins using this pattern.
- Enums use `__attribute__((packed))` (AVR memory sizing).

## Architecture notes
- `main.c` inits rand/gpio/config/lcd, shows the home screen, then loops in
  `set_sleep_mode(SLEEP_MODE_IDLE)`; the CPU sleeps until an interrupt.
- Encoders and buttons are handled in `PCINT0`/`PCINT1` ISRs in `gpio.c`,
  which debounce, decode quadrature, call `ui_manager()` for UI events, and
  feed `rand_add_entropy()` (user-timing entropy). `ui.c` is a state machine
  driven by `UIInput` events.

## Notes for the agent
- The maintainer dislikes "AI cheese" (padded, over-explained prose/comments);
  keep code and comments plain and functional. Refactor-only commits are
  tagged "no functional change".
- No CI, formatter, linter, or typecheck is configured; `.vscode/settings.json`
  enables clang-tidy. Verify changes by building (and running host tests).
