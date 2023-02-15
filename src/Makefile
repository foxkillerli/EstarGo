# Generic compiler options. You probably do not really want to twiddle
# any of this.
# (N.B. -ffast-math breaks us; -fomit-frame-pointer is added below
# unless PROFILING=gprof.)

CC = cc
GPP = g++
CUSTOM_CPPFLAGS =-Wall -fPIC -O3 -pthread -std=c++11 -Wsign-compare -frename-registers
LIBRARIES = -L.
INCLUDES = -I. -I../external/pachi

OBJS = GoBoard.o

all: $(OBJS)
	$(GPP) $(CUSTOM_CPPFLAGS) $(OBJS) -o lib_goboard.so $(LIBRARIES) $(INCLUDES) -shared -Wl,-soname,lib_goboard.so

%.o: %.cpp
	$(GPP) -c -o $@ $< $(CUSTOM_CPPFLAGS) $(INCLUDES)

clean:
	rm -f *.o