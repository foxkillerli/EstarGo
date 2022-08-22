INCLUDES = -I../../include/ -I../../include/utils
GPP = g++
CUSTOM_CPPFLAGS = -Wall -fPIC -O3 -pthread -std=c++11 -Wsign-compare -frename-registers
OBJS = INIReader.o Logger.o Utils.o

all: $(OBJS)

%.o: %.cpp
	$(GPP) -c -o $@ $< $(CUSTOM_CPPFLAGS) $(INCLUDES)
