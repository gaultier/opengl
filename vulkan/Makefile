.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -Wsign-conversion -Wdouble-promotion -g -isystem/usr/local/include -ffast-math -std=c99 #-fsanitize=address
CFLAGS_RELEASE = -O2
LDFLAGS = 
LIBS = -lsdl2 -lvulkan
GLSLC = glslc

.PHONY: all clean

C_FILES= $(wildcard *.c)
H_FILES= $(wildcard *.h)

vulkan_debug: vulkan.c
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LDFLAGS) $(LIBS) $^ -o $@

resources/triangle_vert.spv: resources/triangle.vert
	$(GLSLC) $^ -o $@

resources/triangle_frag.spv: resources/triangle.frag
	$(GLSLC) $^ -o $@

all: vulkan_debug resources/triangle_vert.spv resources/triangle_frag.spv

clean:
	rm vulkan_debug

