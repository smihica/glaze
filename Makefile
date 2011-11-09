.PHONY:	all clean

# macro
LIBGC_PREFIX=/Users/smihica/opt/gc-7.2alpha6
CFLAGS = -I$(LIBGC_PREFIX)/include/gc #-DTRACER
LDFLAGS = -L$(LIBGC_PREFIX)/lib #-DTRACER
CC = gcc
CXX = g++

all:			src/main.o src/object.o src/reader.o src/env.o src/eval.o src/primitives.o src/core.o src/symbol_table.o
			$(CXX) -o bin/arc $^ -lgc $(LDFLAGS)
			strip bin/arc

%.o: %.cc
			$(CXX) -c $(CFLAGS) -o $@ $^

clean:
			rm -rf bin/arc src/*.o
