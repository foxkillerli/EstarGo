# Generic compiler options. You probably do not really want to twiddle
# any of this.
# (N.B. -ffast-math breaks us; -fomit-frame-pointer is added below
# unless PROFILING=gprof.)
CC = gcc
GPP = g++

ROOT_PATH = ..
OUT_PATH = $(ROOT_PATH)
OBJ_DIR =$(ROOT_PATH)
OBJS   =$(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(wildcard *.cpp ./utils/*.cpp)))
PACHI_HOME = $(ROOT_PATH)/external/pachi

CPPFLAGS = -Wall -std=c++11 -O2 -pthread
COMMON_LD_FLAGS = -lmypachi -lIce -lgflags -llog4cplus -lm
INCLUDES=-I/usr/local/include -I$(PACHI_HOME)
ICE_INCLUDES = -DICE_CPP98_MAPPING
LIB_PATH = -L$(PACHI_HOME)

all: $(OBJS)
	$(GPP) $(CPPFLAGS) -o $@ $^ $(LIB_PATH) $(COMMON_LD_FLAGS)

$(OBJ_DIR)/%.o: %.cpp
	$(GPP) $(CPPFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: utils/%.cpp
	$(GPP) $< $(CPPFLAGS) $(INCLUDES) -c -o $@

clean:
	rm -f *.o