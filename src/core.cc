#include "core.h"

#ifndef __linux__

ssize_t fdprintf(int fd, const char *fmt, ...) {
    va_list ap;
    FILE *f = fdopen(fd, "a");
    int rc;

    va_start(ap, fmt);
    rc = vfprintf(f, fmt, ap);
    fclose(f);
    va_end(ap);
    return rc;
}

#endif

void remove_dir(const char* fname, char* buf, size_t size) {
    size_t len = strlen(fname);
    size_t offset  = len;
    for (; offset > 0; offset--) {
        if (fname[offset-1] == '/') break;
    }
    *buf = 0;
    snprintf(buf, size, "%s", fname + offset);

    return;
}
