INCLUDES=-I..
OBJS=dragon.o seki.o 1lib.o 2lib.o nlib.o ladder.o nakade.o selfatari.o util.o

all: $(OBJS)
	$(CC) -c -o $@ $< $(CUSTOM_CFLAGS) $(INCLUDES)

clean:
	rm *.o