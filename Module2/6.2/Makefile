CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -fPIC
LDFLAGS = -Wl,-rpath,.

TARGET = contact_app
LIBRARY = libcontact.so

all: $(TARGET)

$(TARGET): main.c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ main.c -L. -lcontact $(LDFLAGS)

$(LIBRARY): double_list.o contact_book.o
	$(CC) -shared -o $@ $^

double_list.o: double_list.c double_list.h contact_book.h
	$(CC) $(CFLAGS) -c double_list.c -o $@

contact_book.o: contact_book.c contact_book.h double_list.h
	$(CC) $(CFLAGS) -c contact_book.c -o $@

clean:
	rm -rf *.o $(LIBRARY) $(TARGET)

.PHONY: all clean
