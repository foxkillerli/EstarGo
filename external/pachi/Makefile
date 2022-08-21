# Generic compiler options. You probably do not really want to twiddle
# any of this.
# (N.B. -ffast-math breaks us; -fomit-frame-pointer is added below
# unless PROFILING=gprof.)

CC=gcc
CUSTOM_CFLAGS?=-Wall -fPIC -O3 -std=gnu99 -frename-registers -pthread -Wsign-compare -D_GNU_SOURCE
CUSTOM_CXXFLAGS?=-Wall -fPIC -O3

INCLUDES=-I.

OBJS=board.o board_undo.o fbook.o move.o ownermap.o pattern.o pattern3.o patternprob.o patternsp.o random.o stone.o timeinfo.o

SUBDIRS=tactics

all: $(OBJS)
	$(MAKE) -C tactics
	$(CC) -shared -Wl,-soname,libmypachi.so -o libmypachi.so *.o tactics/*.o
	cp libmypachi.so ../../lib/


%.o: %.c
	$(CC) -c -o $@ $< $(CUSTOM_CFLAGS) $(INCLUDES)

clean:
	$(MAKE) -C tactics clean
	rm -f *.so *.o
	rm -f ../../lib/libmypachi.so