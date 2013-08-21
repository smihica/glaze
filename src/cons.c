#include <stdio.h>
#include <stdint.h>
#include "glaze.h"
#include "object.h"
#include "cons.h"

inline glz_cons* cons_new(glz_obj* car, glz_obj* cdr)
{

    return object_cons_new(car, cdr);
}

#define print_list_fpfd(c, fn, fn_arg)                                  \
    {                                                                   \
        ssize_t ret = 0;                                                \
        glz_obj* x    = c->car;                                         \
        glz_obj* next = c->cdr;                                         \
                                                                        \
        if (fn##printf(fn_arg, "(") < 0) err(#fn "printf() failed.");   \
        ret++;                                                          \
                                                                        \
        while(1) {                                                      \
                                                                        \
            ret += object_print_##fn(x, fn_arg);                        \
                                                                        \
            if (!is_cons(next)) {                                       \
                if (!is_nil(next)) {                                    \
                    if (fn##printf(fn_arg, " . ") < 0)                  \
                        err(#fn "printf() failed.");                    \
                    ret += (3 + object_print_##fn(next, fn_arg));       \
                }                                                       \
                break;                                                  \
            }                                                           \
                                                                        \
            /* isCons */                                                \
            if (fn##printf(fn_arg, " ") < 0)                            \
                err(#fn "printf() failed.");                            \
            ret++;                                                      \
                                                                        \
            x    = next->cons.car;                                      \
            next = next->cons.cdr;                                      \
                                                                        \
        }                                                               \
                                                                        \
        if (fn##printf(fn_arg, ")") < 0) err(#fn"printf() failed.");    \
        ret++;                                                          \
                                                                        \
        return ret;                                                     \
    }


inline ssize_t cons_print_fp(glz_cons* c, FILE* fp)
{
    print_list_fpfd(c, fp, fp);
}
