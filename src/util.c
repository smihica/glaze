#include <stdio.h>
#include <string.h>
#include "util.h"

void util_remove_dir(const char* fname, char* buf, size_t size) {
    size_t len = strlen(fname);
    size_t offset  = len;
    for (; offset > 0; offset--) {
        if (fname[offset-1] == '/') break;
    }
    *buf = 0;
    snprintf(buf, size, "%s", fname + offset);
    return;
}

