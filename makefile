CC = gcc
CFLAGS  = -Wall
DEPS  = list.h signal.h

all:	shell

debug: shell.o	list.o signal.o
	$(CC)	$(CFLAGS) -g	-o	shell	shell.o	list.o signal.o
signal.o: signal.c signal.h
	$(CC) $(CFLAGS)		-o	signal.o -c signal.c

list.o:	list.c list.h
	$(CC)	$(CFLAGS)	-o	list.o	-c list.c

shell.o:	shell.c list.h signal.h
	$(CC)	$(CFLAGS)	-o	shell.o	-c	shell.c	

shell:	shell.o	list.o signal.o
	$(CC)	$(CFLAGS)	-o	shell	shell.o	list.o signal.o

clean:
	$(RM)	*.o shell
