.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -g -isystem/usr/local/include -ffast-math -std=c99
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

C_FILES= main.c opengl_lifecycle.c file.c shader.c
H_FILES= opengl_lifecycle.h file.h shader.h utils.h cube.h

opengl: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_FILES) -o opengl

clean:
	rm -f *.o
