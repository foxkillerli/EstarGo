# Generic compiler options. You probably do not really want to twiddle
# any of this.
# (N.B. -ffast-math breaks us; -fomit-frame-pointer is added below
# unless PROFILING=gprof.)

CC = cc
GPP = g++
CUSTOM_CPPFLAGS =-Wall -fPIC -O3 -pthread -std=c++11 -Wsign-compare -frename-registers

INCLUDES=-I. -I../include/ -I../include/utils

OBJS = Config.o GoBoard.o Gtp.o MCTSPlayer.o MCTSTree.o PolicyValueEvaluatorClient.o Proto.o
SUBDIRECTORIES = utils

all: $(OBJS) utils
	$(MAKE) -C utils
	$(GPP) $(CUSTOM_CPPFLAGS) $(OBJS) $(SUBDIRECTORIES)/*.o main.cpp -o eStarGo

%.o: %.cpp
	$(GPP) -c -o $@ $< $(CUSTOM_CPPFLAGS) $(INCLUDES)

clean:
	rm -f *.o