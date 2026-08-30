SHELL = /bin/sh
.SUFFIXES: 

# Project configuration
PROJECT_NAME := ttrpg9000
VERSION_MAJOR := 1
VERSION_MINOR := 3
MCU ?= attiny4313
ISP ?= usbasp
SHADOWRUN ?= 0

# Flags
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Os -mmcu=$(MCU) -I./include
# Size optimizations: link time optimization improves cross function register
# allocation and dead code elimination; mcall-prologues saves program memory
# on the 4KB part by calling shared function prologues. Enable by default;
# build with LTO=0 to disable them (e.g. to bisect a regression).
LTO ?= 1
ifeq (${LTO}, 1)
CFLAGS += -flto -fno-fat-lto-objects -mcall-prologues
endif
# Set SU=1 to emit per function stack usage (.su files) next to the objects
# for stack headroom checks.
ifeq (${SU}, 1)
CFLAGS += -fstack-usage
endif
# Set STACK_GUARD=1 to build the optional on-device stack watermark probe
# (see include/stackguard.h). Read the result back with: make stack-check
ifeq (${STACK_GUARD}, 1)
CFLAGS += -DSTACK_GUARD
endif
ifeq (${SHADOWRUN}, 1)
VARIANT := shadowrun
CFLAGS += -DDEFAULT_GAME_MODE=1
else
VARIANT := standard
endif

# Project directories
# Artifacts are tagged with the MCU and game-mode variant so a SHADOWRUN= or
# MCU= switch lands in a fresh directory and cannot reuse stale objects.
SRCDIR := src
INCDIR := include
BUILDROOT := build
BUILDDIR := $(BUILDROOT)/$(MCU)-$(VARIANT)
DOCS_DIR := docs

# Find files
EXEC = $(BUILDDIR)/$(PROJECT_NAME)_v$(VERSION_MAJOR).$(VERSION_MINOR)
SRCS = $(wildcard $(SRCDIR)/*.c)
HDRS = $(wildcard $(INCDIR)/*.h)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean read-fuses write-fuses flash docs size stack-check

all: $(EXEC).hex

$(EXEC).hex: $(EXEC)
	avr-objcopy -O ihex -j .text -j .data $< $@

$(EXEC): $(OBJS)
	avr-gcc $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HDRS)
	@mkdir -p $(BUILDDIR)
	avr-gcc $(CFLAGS) -c -o $@ $<

docs:
	doxygen Doxyfile

# Report program (flash) and data (RAM) usage against the MCU totals. With a
# SU=1 build, the .su files in the build directory show per function stack
# usage; worst case stack is the deepest call chain plus the ~34 byte ISR
# frame, and headroom is 256 minus data minus that.
size: $(EXEC)
	avr-size -C --mcu=$(MCU) $(EXEC)

# Read back the on-device stack watermark from EEPROM (needs a STACK_GUARD=1
# build that has been running, and the board on the programmer).
stack-check:
	avrdude -q -q -p $(MCU) -c $(ISP) -b 19200 -U eeprom:r:-:r > /tmp/ttrpg_stack.bin
	@od -An -tu1 -N2 /tmp/ttrpg_stack.bin | awk '{ m=($$1>=255)?" (OVERFLOW!)":""; printf "peak stack used = %d bytes%s (of %s bytes RAM)\n", $$1, m, 256 }'

read-fuses:
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U lfuse:r:-:h

write-fuses:
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U lfuse:w:0x64:m
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U hfuse:w:0xDF:m
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U efuse:w:0xFF:m

flash: $(EXEC).hex
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U flash:w:$(EXEC).hex:i

clean:
	rm -rf $(BUILDROOT) $(DOCS_DIR)
