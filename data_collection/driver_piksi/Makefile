CC = clang
CFLAGS = -I. -Wall -g
MAKE = make
DEPS = 
LIBS = libswiftnav-static.a
OBJ = driver_piksi.o
OUT = driver_piksi

.PHONY: all clean

all: $(OBJ) $(LIBS)
	gcc -o $(OUT) $^ $(CFLAGS) $(LIBS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libswiftnav-static.a:
	$(shell cp ../lib/libswiftnav/build/src/libswiftnav-static.a ./)

clean:
	rm *.o $(OUT) || true
	rm $(LIBS) || true
