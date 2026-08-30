#!/usr/bin/env bash
# Thin wrapper around podman for building/flashing the TTRPG-9000 firmware.
# Everything runs inside the Containerfile image with this directory mounted
# at /src, so no AVR toolchain is required on the host.
set -euo pipefail

IMAGE="${IMAGE:-ttrpg9000-builder}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Build the toolchain image only if it is not present yet.
ensure_image() {
    if ! podman image exists "$IMAGE"; then
        echo "==> Building image $IMAGE" >&2
        podman build -t "$IMAGE" -f "$HERE/Containerfile" "$HERE" >&2
    fi
}

usage() {
cat <<'EOF'
Usage: ./container.sh [command] [extra make args...]

Commands:
  build-image        (Re)build the container image
  build              Build the firmware (make)
  shadowrun          Build the Shadowrun firmware (SHADOWRUN=1 make)
  docs               Generate Doxygen documentation (make docs)
  clean              Remove build artifacts (make clean)
  shell              Open a shell inside the container
  flash              Flash the chip with an attached AVR programmer
  shadowrun-flash    Flash the chip with the Shadowrun firmware
  read-fuses         Read the low fuse bits
  write-fuses        Write the fuse bits

Set MCU=... / ISP=... to override the make variables, e.g.:
  ./container.sh build MCU=attiny85314
EOF
}

if [ "$#" -eq 0 ]; then
    usage
    exit 1
fi

cmd="$1"
shift

case "$cmd" in
    -h|--help|help)
        usage
        ;;
    build-image)
        podman build -t "$IMAGE" -f "$HERE/Containerfile" "$HERE"
        ;;
    build)
        ensure_image
        podman run --rm -v "$HERE:/src:Z" "$IMAGE" make "$@"
        ;;
    shadowrun)
        ensure_image
        podman run --rm -e SHADOWRUN=1 -v "$HERE:/src:Z" "$IMAGE" make "$@"
        ;;
    docs|clean)
        ensure_image
        podman run --rm -v "$HERE:/src:Z" "$IMAGE" make "$cmd" "$@"
        ;;
    shell)
        ensure_image
        podman run --rm -it -v "$HERE:/src:Z" "$IMAGE" /bin/bash
        ;;
    flash|read-fuses|write-fuses)
        ensure_image
        # avrdude talks to the USB AVR programmer via libusb, which needs
        # direct device access. --privileged is the simplest way to grant it
        # from a rootless container; see PODMAN.md for safer alternatives.
        podman run --rm -it --privileged -v "$HERE:/src:Z" "$IMAGE" make "$cmd" "$@"
        ;;
    shadowrun-flash)
        ensure_image
        # SHADOWRUN=1 points make at the shadowrun build directory so the
        # firmware flashed matches the one just built.
        podman run --rm -it --privileged -e SHADOWRUN=1 -v "$HERE:/src:Z" "$IMAGE" make flash "$@"
        ;;
    *)
        echo "Unknown command: $cmd" >&2
        usage >&2
        exit 1
        ;;
esac
