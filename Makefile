.PHONY:	all

# macro
CFLAGS = -I/usr/include/gc -L/usr/lib -lgc #-DTRACER

all:		main.o object.o reader.o env.o eval.o primitives.o core.o symbol_table.o
		g++ -o tes main.o object.o reader.o env.o eval.o primitives.o core.o symbol_table.o $(CFLAGS)

main.o:		main.cc
		g++ -c main.cc $(CFLAGS)

object.o:	object.cc object.hh
		g++ -c object.cc $(CFLAGS)

reader.o:	reader.cc reader.hh
		g++ -c reader.cc $(CFLAGS)

eval.o:		eval.cc eval.hh
		g++ -c eval.cc $(CFLAGS)

env.o:		env.cc env.hh
		g++ -c env.cc $(CFLAGS)

primitives.o:	primitives.cc primitives.hh
		g++ -c primitives.cc $(CFLAGS)

core.o:		core.cc core.hh
		g++ -c core.cc $(CFLAGS)

symbol_table.o:	symbol_table.cc symbol_table.hh
		g++ -c symbol_table.cc $(CFLAGS)

clean:
		rm tes main.o object.o reader.o eval.o env.o primitives.o core.o symbol_table.o
