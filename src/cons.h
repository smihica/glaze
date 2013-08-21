#ifndef GLAZE__CONS_H_
#define GLAZE__CONS_H_

#include <stdint.h>

#define list_new(a)              cons_new(a, glz_nil)
#define list_new2(a, b)          cons_new(a, (glz_obj*)list_new(b))
#define list_new3(a, b, c)       cons_new(a, (glz_obj*)list_new2(b, c))
#define list_new4(a, b, c, d)    cons_new(a, (glz_obj*)list_new3(b, c, d))
#define list_new5(a, b, c, d, e) cons_new(a, (glz_obj*)list_new4(b, c, d, e))

extern inline glz_cons* cons_new(glz_obj* car, glz_obj* cdr);
extern inline ssize_t cons_print_fp(glz_cons* c, FILE* fp);

#endif
