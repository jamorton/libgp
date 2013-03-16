
# Library
LIB_SOURCES=src/world.c src/program.c
LIB_INCLUDES=$(wildcard include/*.h) $(wildcard src/*.h)
LIB_OUT=libgp.a

# Examples
EXAMPLES_SOURCES=$(wildcard examples/*.c)

# Docs
DOCS_SOURCES = src/gp.c $(EXAMPLES_SOURCES)

#=============================================================================#

WARNINGS=-Wall -Wextra -Wshadow -pedantic -Werror \
         -Wstrict-prototypes -Wmissing-prototypes \
         -Wno-unused-parameter -Wno-unused-function \
         -Wno-unused-variable

CC=gcc
CFLAGS=$(WARNINGS) -std=c99 -O3 -Iinclude
DEBUG_CFLAGS=-g -DDEBUG

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CFLAGS:=$(CFLAGS) $(DEBUG_CFLAGS)
endif

LIB_OBJECTS=$(LIB_SOURCES:%.c=out/%.o)
EXAMPLES_OBJECTS=$(basename $(EXAMPLES_SOURCES))

#=============================================================================#

all: $(LIB_OUT) $(EXAMPLES_OBJECTS)

$(LIB_OUT): $(LIB_OBJECTS)
	ar rcs $@ $(LIB_OBJECTS)

$(LIB_OBJECTS): out/%.o: %.c $(LIB_INCLUDES)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXAMPLES_OBJECTS): %: %.c $(LIB_OUT)
	$(CC) $(CFLAGS) -L. -lgp $< -o $@

clean:
	rm -f libgp.a
	rm -f docs/*.html
	rm -rf $(LIB_OBJECTS)
	rm -f $(EXAMPLES_OBJECTS)

docs:
	docco $(DOCS_SOURCES)

.PHONY: clean docs
