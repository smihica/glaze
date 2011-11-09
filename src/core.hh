#ifndef CORE_H_
#define CORE_H_

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <vector>

/* for BoehmGC */
#define GC_DEBUG
#include "gc.h"
#include "gc_cpp.h"
#define CHECK_LEAKS() GC_gcollect()

#define array_sizeof(a) ((int)(sizeof(a)/sizeof(a[0])))
#define CALLERROR(...) error(__FILE__, __LINE__, __VA_ARGS__)

#ifdef __linux__

#define fdprintf(...) dprintf(__VA_ARGS__)
#else // freebsd or MacOS

ssize_t fdprintf(int fd, const char *fmt, ...);
#endif

#endif
