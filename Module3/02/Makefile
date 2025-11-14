CC = gcc
SOURCES = findMax.c findMaxLen.c main.c
TARGETS = findMax findMaxLen main

all: $(TARGETS)

findMax: findMax.c
	$(CC) -o $@ $<

findMaxLen: findMaxLen.c
	$(CC) -o $@ $<

main: main.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGETS)

.PHONY: all clean