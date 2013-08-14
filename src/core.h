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
#define ERR(...) error(__FILE__, __LINE__, __VA_ARGS__)

#define REQUIRE_GC_

#ifdef REQUIRE_GC_

#define GC_DEBUG
#define CHECK_LEAKS() GC_gcollect()

#ifdef __linux__
#include <gc/gc.h>
#include <gc/gc_cpp.h>
#include <gc/gc_allocator.h>
#else // freebsd or MacOS
#include <gc.h>
#include <gc_cpp.h>
#include <gc/gc_allocator.h>
int fdprintf(int fd, const char *fmt, ...);
#endif

#endif // REQUIRE_GC_

#ifdef __linux__
#define fdprintf(...) dprintf(__VA_ARGS__)
#else // freebsd or MacOS
int fdprintf(int fd, const char *fmt, ...);
#endif

#define READ_STRING_SMALL_BUFFER_SIZE   1024
#define READ_NUMBER_BUFFER_SIZE         256
#define MAX_READ_SYMBOL_LENGTH          256

void remove_dir(const char* fname, char* buf, size_t size);

#endif // GLAZE__CORE_H_
