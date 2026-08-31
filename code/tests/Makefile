SHELL = /bin/sh
.SUFFIXES:

# Host unit tests for the pure math helpers in include/mathutil.h.
# These build with the system compiler (not avr-gcc) and run natively.
# They are kept separate from the firmware build so nothing collides.

CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2 -I../include

BUILDDIR := build
MATHUTIL_H := ../include/mathutil.h
TESTS := test_mathutil

.PHONY: all run clean

all: $(addprefix $(BUILDDIR)/,$(TESTS))

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%: %.c $(MATHUTIL_H) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $<

# Build and run every test; exit non zero if any test fails.
run: all
	@status=0; \
	for t in $(TESTS); do \
		echo "== $$t =="; \
		$(BUILDDIR)/$$t || status=1; \
	done; \
	exit $$status

clean:
	rm -rf $(BUILDDIR)
