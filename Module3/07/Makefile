CC = gcc
SOURCES = client_a.c client_b.c
TARGETS = client_a client_b
LDFLAGS = -lrt

all: $(TARGETS)

client: client.c
	$(CC) $(LDFLAGS) -o $@ $<

main: main.c
	$(CC) $(LDFLAGS) -o $@ $<

clean:
	rm -rf $(TARGETS)

.PHONY: all clean