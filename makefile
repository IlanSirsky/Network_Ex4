OUTSERVER = measure
OUTCLIENT = sender
CC = gcc
CFLAGS = -Wall -g

all: server client

measure.o: measure.c
	$(CC) -c $(CFLAGS) measure.c

sender.o: sender.c
	$(CC) -c $(CFLAGS) sender.c

server: measure.o
	$(CC) $(CFLAGS) -o $(OUTSERVER) measure.o

client: sender.o
	$(CC) $(CFLAGS) -o $(OUTCLIENT) sender.o

.PHONY: all clean output server client

clean:
	rm -f *.o $(OUTSERVER) $(OUTCLIENT)