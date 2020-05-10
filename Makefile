.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -g -isystem/usr/local/include -ffast-math -std=c99
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

C_FILES= $(wildcard *.c)
H_FILES= $(wildcard *.h)

opengl: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_FILES) -o opengl

clean:
	rm -f *.o opengl
