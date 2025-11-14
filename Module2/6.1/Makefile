CC = gcc
AR=ar
ARFLAFS=rc

SRCS= double_list.c contact_book.c
OBJS= double_list.o contact_book.o

TARGET = contact_app
LIBRARY = libcontact.a

all: $(TARGET)

$(LIBRARY): $(OBJS)
	$(AR) $(ARFLAFS) $@ $^

%.o: %.c
	$(CC) -c $< -o $@

$(TARGET): $(LIBRARY) main.c
	$(CC) -o $@ main.c -L. -lcontact

clean:
	rm -rf $(OBJS) $(LIBRARY) $(TARGET)

PHONY: all clean