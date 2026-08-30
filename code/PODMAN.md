# Building the TTRPG-9000 firmware in a Podman container

This directory ships a `Containerfile` that packages the whole AVR toolchain (`avr-gcc`, `avrdude`) along with other needed build tools so you can build the firmware without installing anything on the host beyond Podman. The source tree is *mounted* into the image at run time, so the image only has to be built once and never needs to be rebuilt when the firmware changes.

## Requirements

- [Podman](https://podman.io/) installed and able to run rootless (the default). Docker also works, but the commands here assume `podman`.
- Network access the first time, to pull `debian:bookworm-slim` and install the toolchain packages.

## Quick start

From this directory:

```bash
# Standard firmware build -> build/ttrpg9000_v1.3.hex
./container.sh build

# Shadowrun firmware build
./container.sh shadowrun

# API documentation -> docs/html/index.html
./container.sh docs
```

The first run builds the container image automatically (takes a minute);
subsequent runs reuse it.

## The helper script

`container.sh` is a thin wrapper that mounts this directory into the image and
runs the matching `make` target. Available commands:

| Command               | Does                                              |
| --------------------- | ------------------------------------------------- |
| `build`               | `make` — standard firmware                        |
| `shadowrun`           | `SHADOWRUN=1 make` — Shadowrun firmware           |
| `docs`                | `make docs` — Doxygen HTML docs                   |
| `clean`               | `make clean` — remove build/docs artifacts        |
| `shell`               | interactive shell inside the container            |
| `flash`               | `make flash` — write firmware via AVR programmer  |
| `read-fuses`          | `make read-fuses`                                 |
| `write-fuses`         | `make write-fuses`                                |
| `build-image`         | (re)build the container image                     |

Extra arguments are forwarded to `make`, so you can override the make
variables the same way you would on the host:

```bash
./container.sh build MCU=attiny4313
./container.sh flash MCU=attiny4313 ISP=usbasp
```

Run `./container.sh help` for the same summary.
