#include <stdio.h>
#include <stdint.h>
#include "glaze.h"
#include "object.h"
#include "string.h"
#include "symbol.h"

inline glz_symbol* symbol_new(const char* raw, size_t l)
{
    glz_string* name = string_new(raw, l);

    if (name->tt != GLZ_TT_SSTRING)
        err("symbol must be shorter than %d bytes", GLZ_SYMBOL_LIMIT - 1);

    return symbol_symbolize(name);
}

inline int symbol_compare(glz_symbol* s1, glz_symbol* s2)
{
    return (s1 == s2);
}

inline ssize_t symbol_print_fp(glz_symbol* s, FILE* fp)
{
    return (ssize_t)fprintf(fp, "%s", (const char*)((&(s->name))+1));
}
