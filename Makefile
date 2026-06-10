CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2
TARGET := rtc_check

.PHONY: all clean

all: $(TARGET)

$(TARGET): src/rtc_check.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

