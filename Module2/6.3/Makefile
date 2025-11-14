CC = gcc
LDFLAGS = -ldl -Wl,-rpath,'$$ORIGIN'
SHARED_FLAGS = -shared -fPIC

TARGET = calculator
LIBS = libadd.so libsub.so libmul.so libdiv.so

all: $(LIBS) $(TARGET)

$(TARGET): main.c calculate.c
	$(CC) -o $@ $^ $(LDFLAGS)

%.so: %.c
	$(CC) $(SHARED_FLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(LIBS)

run: all
	./$(TARGET)

rebuild: clean all

help:
	@echo "Доступные команды:"
	@echo "  make all     - собрать проект"
	@echo "  make run     - собрать и запустить"
	@echo "  make clean   - очистить проект"
	@echo "  make rebuild - пересобрать"
	@echo "  make help    - показать справку"

.PHONY: all clean run rebuild help