#ifndef GLAZE__SYMBOL_H_
#define GLAZE__SYMBOL_H_

#include <stdint.h>

#define symbol_symbolize(str) \
    ((glz_symbol*)(((char*)str) - (GLZ_SYMBOL_SIZE - GLZ_STRING_SIZE)))

extern inline glz_symbol* symbol_new(const char *name, size_t l);
extern inline int symbol_compare(glz_symbol* s1, glz_symbol* s2);
extern inline ssize_t symbol_print_fp(glz_symbol* s, FILE* fp);

#endif
