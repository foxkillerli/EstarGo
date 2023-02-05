all: ice

ice:
	slice2cpp --impl-c++98 Proto.ice
	#slice2py Proto.ice
	#gsed -i 's@#include <Proto.h>@#include "../include/Proto.h"@g' Proto.cpp
	sed -i 's@#include <Proto.h>@#include "../include/Proto.h"@g' Proto.cpp
	cp Proto.h ../include/
	cp Proto.cpp ../src/

clean:
	rm -f Proto_ice.py
	rm -f Proto.h
	rm -f Proto.cpp
	rm -f ProtoI.h
	rm -f ProtoI.cpp
	rm -f ../include/Proto.h
	rm -f ../src/Proto.cpp
	rm -f *.o
