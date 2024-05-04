HEX=./build/main.hex
OBJ=./build/main.o
ISP?=usbtiny

.PHONY: clean read-fuses write-fuses flash

all: $(HEX)

$(OBJ): src/main.c
	@mkdir -p build
	avr-gcc -Wall -Os -mmcu=attiny2313a -o $@ $<

$(HEX): $(OBJ)
	@mkdir -p build
	avr-objcopy -O ihex -j .text -j .data $< $@

read-fuses:
	avrdude -v -p attiny2313 -c $(ISP) -b 19200 -U lfuse:r:-:h

write-fuses:
	avrdude -v -p t2313 -c $(ISP) -b 19200 -U lfuse:w:0x64:m
	avrdude -v -p t2313 -c $(ISP) -b 19200 -U hfuse:w:0xDF:m
	avrdude -v -p t2313 -c $(ISP) -b 19200 -U efuse:w:0xFF:m

flash: $(HEX)
	avrdude -v -p t2313 -c $(ISP) -b 19200 -U flash:w:$(HEX):i

clean:
	rm -rf build/*
