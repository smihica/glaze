.PHONY:	all

# macro
CFLAGS = -I/usr/include/gc -L/usr/lib -lgc #-DTRACER

all:			src/main.o src/object.o src/reader.o src/env.o src/eval.o src/primitives.o src/core.o src/symbol_table.o
			g++ -o bin/arc src/main.o src/object.o src/reader.o src/env.o src/eval.o src/primitives.o src/core.o src/symbol_table.o $(CFLAGS)

src/main.o:		src/main.cc
			g++ -c src/main.cc -o src/main.o $(CFLAGS)

src/object.o:		src/object.cc src/object.hh
			g++ -c src/object.cc -o src/object.o $(CFLAGS)

src/reader.o:		src/reader.cc src/reader.hh
			g++ -c src/reader.cc -o src/reader.o $(CFLAGS)

src/eval.o:		src/eval.cc src/eval.hh
			g++ -c src/eval.cc -o src/eval.o $(CFLAGS)

src/env.o:		src/env.cc src/env.hh
			g++ -c src/env.cc -o src/env.o $(CFLAGS)

src/primitives.o:	src/primitives.cc src/primitives.hh
			g++ -c src/primitives.cc -o src/primitives.o $(CFLAGS)

src/core.o:		src/core.cc src/core.hh
			g++ -c src/core.cc -o src/core.o $(CFLAGS)

src/symbol_table.o:	src/symbol_table.cc src/symbol_table.hh
			g++ -c src/symbol_table.cc -o src/symbol_table.o $(CFLAGS)

clean:
			rm -rf bin/arc src/main.o src/object.o src/reader.o src/eval.o src/env.o src/primitives.o src/core.o src/symbol_table.o
