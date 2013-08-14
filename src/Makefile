OBJECTS=$(shell ls *.cc | sed -e 's/\.cc/\.o/g')
CC=gcc
CXX=g++
LDFLAGS+=-lgc

.PHONY:			all clean

all:			init glaze

%.o:			%.cc
			$(CXX) -c $(CFLAGS) -o $@ $^

glaze:			$(OBJECTS)
			$(CXX) $(LDFLAGS) -o glaze $^

clean:
			rm -rf glaze version.h char_funcs_tbl.h *.o lib/*.core

init:
			./generators/_generate
			lib/converter
