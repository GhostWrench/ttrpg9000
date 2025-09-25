SHELL = /bin/sh
.SUFFIXES: 

# Project configuration
PROJECT_NAME := ttrpg9000
VERSION_MAJOR := 1
VERSION_MINOR := 2
MCU ?= attiny4313
ISP ?= usbasp
SHADOWRUN ?= 0

# Flags
CFLAGS := -Wall -Wextra -Wpedantic -Os -mmcu=$(MCU) -I./include
ifeq (${SHADOWRUN}, 1)
CFLAGS += -DDEFAULT_GAME_MODE=1
endif

# Project directories
SRCDIR := src
INCDIR := include
BUILDDIR := build

# Find files
EXEC = $(BUILDDIR)/$(PROJECT_NAME)_v$(VERSION_MAJOR).$(VERSION_MINOR)
SRCS = $(wildcard $(SRCDIR)/*.c)
HDRS = $(wildcard $(INCDIR)/*.h)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: clean read-fuses write-fuses flash

all: $(EXEC).hex

$(EXEC).hex: $(EXEC)
	avr-objcopy -O ihex -j .text -j .data $< $@

$(EXEC): $(OBJS)
	avr-gcc $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HDRS)
	@mkdir -p $(BUILDDIR)
	avr-gcc $(CFLAGS) -c -o $@ $<

read-fuses:
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U lfuse:r:-:h

write-fuses:
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U lfuse:w:0x64:m
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U hfuse:w:0xDF:m
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U efuse:w:0xFF:m

flash: $(EXEC).hex
	avrdude -v -p $(MCU) -c $(ISP) -b 19200 -U flash:w:$(EXEC).hex:i

clean:
	rm -rf build/*
