OBJS = MYSHELL
CC = gcc
CFLAGS = -Wall

.PHONY : MYSHELL
myshell : $(OBJS)
	$(CC) $(CFLAGS) myshell.c -o MYSHELL
clean :
	rm MYSHELL
