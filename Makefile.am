## lib
GLAZE_LIB_SRCS =	\
src/core.cc		\
src/object.cc		\
src/env.cc		\
src/eval.cc		\
src/gc.cc		\
src/gc.h		\
src/primitives.cc	\
src/primitives.h	\
src/reader.cc		\
src/symbol_table.cc	\
src/symbol_table.h	\
src/glaze.cc

GLAZE_LIB_HDRS =	\
src/core.h		\
src/object.h		\
src/env.h		\
src/eval.h		\
src/reader.h		\
src/shared.h		\
src/glaze.h

lib_LTLIBRARIES = libglaze.la
libglaze_ladir=$(includedir)/glaze
libglaze_la_CPPFLAGS =
libglaze_la_LDFLAGS = -lgc
libglaze_la_HEADERS = $(GLAZE_LIB_HDRS)
libglaze_la_SOURCES = $(GLAZE_LIB_SRCS)

## bin
bin_PROGRAMS = glaze
GLAZE_CORE_SRCS = 	\
src/glaze.h		\
src/main.cc

glaze_LDADD = libglaze.la
glaze_SOURCES = $(GLAZE_CORE_SRCS)
glaze_LDFLAGS = -static
