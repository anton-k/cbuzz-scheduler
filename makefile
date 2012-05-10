cc = gcc
cflags = -ggdb -c -Wall

	
all: pqueue.o time.o rts.o
	$(cc) -ggdb $^ main.c	
	./a.out

%.o: %.c
	$(cc) $(cflags) $< -o $@

	
