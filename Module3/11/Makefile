CC = gcc
LDFLAGS = -lrt -lpthread
SRCS = main.c
TARGET = shm

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -rf $(TARGET)

.PHONY: all clean