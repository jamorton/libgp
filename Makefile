
WARNINGS=-Wall -Wextra -Wpointer-arith -Wshadow \
         -Wstrict-prototypes -Wmissing-prototypes \
         -Wno-unused-parameter -Wno-unused-function \
         -Wno-unused-variable

CC=gcc
CFLAGS=$(WARNINGS) -std=c99 -O3 -Iinclude

bin/libcgp.a: src/gp.c
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

doc:
	docco src/gp.c
