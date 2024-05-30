CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SOURCES = disk.c filesystem.c shell.c
HEADERS = disk.h filesystem.h
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = miniFS
DISK_IMAGE = disk.img
DISK_SIZE = 100M

all: $(EXECUTABLE) $(DISK_IMAGE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

$(DISK_IMAGE):
	dd if=/dev/zero of=$(DISK_IMAGE) bs=1M count=100

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(DISK_IMAGE)

.PHONY: all clean