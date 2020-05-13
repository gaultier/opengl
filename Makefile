.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -g -isystem/usr/local/include -ffast-math -std=c99
CFLAGS_RELEASE = -O2
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

C_FILES= $(wildcard *.c)
H_FILES= $(wildcard *.h)

opengl_debug: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_FILES) -o $@

opengl_release: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LDFLAGS) $(C_FILES) -o $@


vulkan_debug: vulkan/vulkan.c
	$(CC) $(CFLAGS) $(CFLAGS_RELEASE) $(LDFLAGS) $^ -o $@

clean:
	rm -f *.o opengl_debug opengl_release vulkan_debug
