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

.PHONY: all clean read-fuses write-fuses flash docs

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
