.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic
LDFLAGS = -lsdl2

.PHONY: clean

opengl: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) main.c -o opengl

clean:
	rm -f *.o
