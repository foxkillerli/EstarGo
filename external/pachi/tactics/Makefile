INCLUDES=-I..
OBJS = dragon.o seki.o 1lib.o 2lib.o nlib.o ladder.o nakade.o selfatari.o util.o
CC = gcc
CFLAGS = -Wall -fPIC -O3 -std=gnu99 -frename-registers -pthread -Wsign-compare -D_GNU_SOURCE

all: lib.a
lib.a: $(OBJS)
	$(CC) -c -o $@ $< $(CUSTOM_CFLAGS) $(INCLUDES)

clean:
	rm *.o