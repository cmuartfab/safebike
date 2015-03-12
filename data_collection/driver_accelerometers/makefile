CC=gcc
CFLAGS=-I.
TARGET=driver_accelerometers

all: main.o slipstream.o slipstream_server
	gcc -o $(TARGET) main.o slipstream.o -I.

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

slipstream_server: slipstream_server.o
	$(CC) -o slipstream_server slipstream_server.o -I. -lpthread

clean: 
	rm -f *.o *~ core $(TARGET) slipstream_server

