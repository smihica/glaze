#ifndef GLAZE__CORE_H_
#define GLAZE__CORE_H_

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>

#include <vector>
#include <map>
#include <string>

#define array_sizeof(a) ((int)(sizeof(a)/sizeof(a[0])))
#define CALLERROR(...) error(__FILE__, __LINE__, __VA_ARGS__)

#define GC_DEBUG

#ifdef __linux__
#include <gc/gc.h>
#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>

#define fdprintf(...) dprintf(__VA_ARGS__)

#else // freebsd or MacOS
#include <gc.h>
#include <gc_cpp.h>
#include <gc/gc_allocator.h>

ssize_t fdprintf(int fd, const char *fmt, ...);
#endif

/* for BoehmGC */
#define CHECK_LEAKS() GC_gcollect()

#define READ_STRING_SMALL_BUFFER_SIZE   1024
#define READ_NUMBER_BUFFER_SIZE         256
#define MAX_READ_SYMBOL_LENGTH          256

void remove_dir(const char* fname, char* buf, size_t size);

#endif // GLAZE__CORE_H_
