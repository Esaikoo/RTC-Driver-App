CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Iinclude
TARGET := rtc_check

.PHONY: all clean version

all: $(TARGET)

$(TARGET): src/rtc_check.c
	$(CC) $(CFLAGS) -o $@ $<

version:
	@grep ATLAS_RTC_DEMO_VERSION include/project_version.h

clean:
	rm -f $(TARGET)
