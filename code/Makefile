HEX=build/main.hex
OBJ=build/main.o
DEV?=/dev/ttyACM0
ISP?=stk500v1

.PHONY: clean fuses flash

all: $(HEX)

$(OBJ): src/main.c
	avr-gcc -mmcu=attiny2313a -o $@ $<

$(HEX): $(OBJ)
	avr-objcopy -O ihex -j .text -j .data $< $@

read-fuses:
	avrdude -v -p attiny2313 -c $(ISP) -P $(DEV) -U -b 19200 lfuse:r:-:m

write-fuses:
	@echo "Not implemented"

flash:
	@echo "not implemented"

clean:
	rm -rf build/*
