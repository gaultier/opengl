.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic -g
LDFLAGS = -lsdl2 -framework OpenGL 

.PHONY: clean

C_FILES= main.c opengl_lifecycle.c file.c malloc.c shader.c
H_FILES= opengl_lifecycle.h file.h malloc.h shader.h utils.h

opengl: $(C_FILES) $(H_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(C_FILES) -o opengl

clean:
	rm -f *.o
