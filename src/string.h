#ifndef GLAZE__STRING_H_
#define GLAZE__STRING_H_

#include <stdint.h>
#include "string_hash.h"

#define string_raw(stro)  ((const char *)((stro)+1))
extern glz_string_hash* string_table;

void string_init();
void string_exit();
glz_string* string_new(const char *str, size_t l);
extern inline int string_lstring_compare(glz_string* s1, glz_string* s2);
extern inline int string_compare(glz_string* s1, glz_string* s2);
extern inline ssize_t string_print_fp(glz_string* s, FILE* fp);

#endif
