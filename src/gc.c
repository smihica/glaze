#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "glaze.h"
#include "object.h"
#include "gc.h"

void* gc_realloc(void* ob, size_t os, size_t ns)
{
    (void) os; // unused
    if (ns == 0) {
        free(ob);
        return NULL;
    }
    return realloc(ob, ns);
}

void* gc_newblock(void *ob, size_t os, size_t ns)
{
    // size_t ns = (ob) ? os : 0;
    void* nb = gc_realloc(ob, os, ns);
    if (nb == NULL && ns > 0) {
        fatal("realloc failed memory exhausted.");
    }
    return nb;
}
