.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

opengl: main.c opengl_lifecycle.c opengl_lifecycle.h
	$(CC) $(CFLAGS) $(LDFLAGS) main.c opengl_lifecycle.c -o opengl

clean:
	rm -f *.o
