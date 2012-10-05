
WARNINGS=-Wall -Wextra -Wshadow -pedantic -Werror \
         -Wstrict-prototypes -Wmissing-prototypes \
         -Wno-unused-parameter -Wno-unused-function \
         -Wno-unused-variable

CC=gcc
CFLAGS=$(WARNINGS) -std=c99 -O3 -Iinclude
DEBUG_CFLAGS=-g -DDEBUG
LIB_SOURCES=src/gp.c
LIB_INCLUDES=$(wildcard include/*.h) $(wildcard src/*.h)

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS:=$(CFLAGS) $(DEBUG_CFLAGS)
endif

bin/libcgp.a: $(LIB_SOURCES) $(LIB_INCLUDES)
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f bin/libcgp.a

doc:
	docco src/gp.c
