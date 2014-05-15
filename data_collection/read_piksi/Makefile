CC = gcc
CFLAGS = -I. -Wall
MAKE = make
DEPS = 
LIBS = libswiftnav-static.a
OBJ = read_piksi.o
OUT=read_piksi

.PHONY: all

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libswiftnav-static.a:
	$(shell cp ../lib/libswiftnav/build/src/libswiftnav-static.a ./)

all: $(OBJ) $(LIBS)
	gcc -o $(OUT) $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o $(OUT) || true
	rm $(LIBS) || true
