CC = gcc
CFLAGS = -Wall -Wextra -g
TARGETS = cli driver

all: $(TARGETS)

cli: cli.c common.h
	$(CC) $(CFLAGS) -o cli cli.c

driver: driver.c common.h
	$(CC) $(CFLAGS) -o driver driver.c

clean:
	rm -f $(TARGETS) /tmp/taxi_system.sock

run: all
	./cli