CC = gcc
SOURCES = producer.c consumer.c
TARGETS = producer consumer

all: $(TARGETS)

producer: producer.c
	$(CC) -o $@ $<

consumer: consumer.c
	$(CC) -o $@ $<

clean:
	rm -rf $(TARGETS)

.PHONY: all clean