
WARNINGS=-Wall -Wextra -Wpointer-arith -Wshadow \
         -Wstrict-prototypes -Wmissing-prototypes \
         -Wno-unused-parameter -Wno-unused-function \
         -Wno-unused-variable

CC=gcc
CFLAGS=$(WARNINGS) -std=c99 -O3 -Iinclude
LIB_SOURCES=src/gp.c

bin/libcgp.a: $(LIB_SOURCES)
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f bin/libcgp.a

doc:
	docco src/gp.c
