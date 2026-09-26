SHELL = /bin/sh
.SUFFIXES:

# Unit tests that can be run on the host computer

CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2 -I../include -DHOST_BUILD

BUILDDIR := build
HDRS := $(wildcard ../include/*.h)
TESTS_C := $(wildcard *.c)
TESTS := $(patsubst %.c,build/%,$(TESTS_C))

.PHONY: all clean

all: $(TESTS)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

build/rand: rand.c ../src/rand.c $(HDRS) $(BUILDDIR)
	$(CC) $(CFLAGS) rand.c ../src/rand.c -o $@

build/queue: queue.c ../src/queue.c $(HDRS) $(BUILDDIR)
	$(CC) $(CFLAGS) queue.c ../src/queue.c -o $@

run: all
	@status=0; \
	for t in $(TESTS); do \
		echo "== $$t =="; \
		$$t || status=1; \
	done; \
	exit $$status

clean:
	rm -rf build
