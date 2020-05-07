.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

opengl: main.c opengl_lifecycle.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o opengl

clean:
	rm -f *.o
