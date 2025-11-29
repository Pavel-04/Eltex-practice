CC = gcc
SOURCES = server.c client.c raw.c
TARGETS = server client raw

all: $(TARGETS)

client: client.c
	$(CC) -o $@ $<

server: server.c
	$(CC) -o $@ $<

raw: raw.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGETS)

.PHONY: all clean