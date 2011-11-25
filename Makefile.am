bin_PROGRAMS = glaze

GLAZE_CORE_SRCS =	\
src/core.cc		\
src/core.h		\
src/object.cc		\
src/object.h		\
src/shared.h		\
src/env.cc		\
src/env.h		\
src/eval.cc		\
src/eval.h		\
src/gc.cc		\
src/gc.h		\
src/primitives.cc	\
src/primitives.h	\
src/reader.cc		\
src/reader.h		\
src/symbol_table.cc	\
src/symbol_table.h	\
src/glaze.cc		\
src/glaze.h		\
src/main.cc

glaze_CPPFLAGS =

glaze_LDFLAGS = -lgc -lmpc

glaze_SOURCES = $(GLAZE_CORE_SRCS)