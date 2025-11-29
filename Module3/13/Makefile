CC = gcc
SOURCES = server.c client.c
TARGETS = server client

all: $(TARGETS)

client: client.c
	$(CC) -o $@ $<

main: main.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGETS)

.PHONY: all clean