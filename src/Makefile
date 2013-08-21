OBJECTS=$(shell ls *.c | sed -e 's/\.c/\.o/g')
CC=gcc
CXX=g++
# LDFLAGS+=-lgc
CFLAGS+=-O3

.PHONY:			all clean

all:			generate glz

%.o:			%.c
			$(CC) -c $(CFLAGS) -o $@ $^

glz:			$(OBJECTS)
			$(CC) $(LDFLAGS) -o glz $^

clean:
			rm -rf *.o glz

generate:
			./gen/gen

#init:
#			./generators/_generate
#			lib/converter
