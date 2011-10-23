.PHONY:	all

# macro
LIBGC_PREFIX=
CFLAGS = -I$(LIBGC_PREFIX)/include/gc #-DTRACER
LDFLAGS = -L$(LIBGC_PREFIX)/lib #-DTRACER
CC = gcc
CXX = g++

all:			src/main.o src/object.o src/reader.o src/env.o src/eval.o src/primitives.o src/core.o src/symbol_table.o
			$(CXX) -o bin/arc src/main.o src/object.o src/reader.o src/env.o src/eval.o src/primitives.o src/core.o src/symbol_table.o -L/usr/lib -lgc $(LDFLAGS)

src/main.o:		src/main.cc
			$(CXX) -c src/main.cc -o src/main.o $(CFLAGS)

src/object.o:		src/object.cc src/object.hh
			$(CXX) -c src/object.cc -o src/object.o $(CFLAGS)

src/reader.o:		src/reader.cc src/reader.hh
			$(CXX) -c src/reader.cc -o src/reader.o $(CFLAGS)

src/eval.o:		src/eval.cc src/eval.hh
			$(CXX) -c src/eval.cc -o src/eval.o $(CFLAGS)

src/env.o:		src/env.cc src/env.hh
			$(CXX) -c src/env.cc -o src/env.o $(CFLAGS)

src/primitives.o:	src/primitives.cc src/primitives.hh
			$(CXX) -c src/primitives.cc -o src/primitives.o $(CFLAGS)

src/core.o:		src/core.cc src/core.hh
			$(CXX) -c src/core.cc -o src/core.o $(CFLAGS)

src/symbol_table.o:	src/symbol_table.cc src/symbol_table.hh
			$(CXX) -c src/symbol_table.cc -o src/symbol_table.o $(CFLAGS)

clean:
			rm -rf bin/arc src/main.o src/object.o src/reader.o src/eval.o src/env.o src/primitives.o src/core.o src/symbol_table.o
