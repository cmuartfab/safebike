CC=gcc
CFLAGS=-I. -Wall
DEPS = 
LIBS = libswiftnav-static.a
OBJ = read_piksi.o
OUT=read_piksi

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	gcc -o $(OUT) $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o $(OUT) || true