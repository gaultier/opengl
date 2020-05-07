.POSIX:

CFLAGS = -Wall -Wextra -Wpedantic

.PHONY: clean

opengl:
	$(CC) $(CFLAGS) main.c -o main.o

clean:
	rm -f *.o
