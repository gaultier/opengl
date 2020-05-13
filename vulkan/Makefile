.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -g -isystem/usr/local/include -ffast-math -std=c99
CFLAGS_RELEASE = -O2
LDFLAGS = 
LIBS = -lsdl2

.PHONY: clean

C_FILES= $(wildcard *.c)
H_FILES= $(wildcard *.h)

vulkan/vulkan_debug: vulkan/vulkan.c
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LDFLAGS) -lsdl2 $^ -o $@

clean:
	rm vulkan_debug

