#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include "glaze.h"
#include "util.h"
#include "object.h"
#include "reader.h"
#include "string.h"
#include "symbol.h"

void glz_error(const char* fname, unsigned int line, const char* fmt, ...)
{
    char fname_buf[32];
    util_remove_dir(fname, fname_buf, 32);

    va_list arg;
    va_start(arg, fmt);

    fprintf(stderr, "%s:%u error: ", fname_buf, line);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    fflush(stderr);

    va_end(arg);
}

void glz_bug(const char* fname, unsigned int line, const char* fmt, ...)
{
    char fname_buf[32];
    util_remove_dir(fname, fname_buf, 32);

    va_list arg;
    va_start(arg, fmt);

    fprintf(stderr, "%s:%u bug: ", fname_buf, line);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    fflush(stderr);

    va_end(arg);

    exit(1);
}

void glz_fatal(const char* fname, unsigned int line, const char* fmt, ...)
{
    char fname_buf[32];
    util_remove_dir(fname, fname_buf, 32);

    va_list arg;
    va_start(arg, fmt);

    fprintf(stderr, "%s:%u fatal: ", fname_buf, line);
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    fflush(stderr);

    va_end(arg);

    exit(1);
}

int obj_tests()
{
    glz_header  header;
    glz_number  num;
    glz_string  str;
    glz_symbol  sym;
    glz_cons    con;
    glz_table   tbl;
    glz_closure closure;
    glz_regexp  reg;
    glz_thread  thread;
    glz_tagged  tagged;
    glz_obj     obj;
    glz_value   nil;
    glz_value   t;
    glz_value   fix;
    glz_value   flo;

    printf("checking size\n");
    printf("header:  %zd, %d\n", sizeof(header)  , GLZ_HEADER_SIZE);
    printf("number:  %zd, %d\n", sizeof(num)     , GLZ_NUMBER_SIZE);
    printf("string:  %zd, %d\n", sizeof(str)     , GLZ_STRING_SIZE);
    printf("symbol:  %zd, %d\n", sizeof(sym)     , GLZ_SYMBOL_SIZE);
    printf("cons:    %zd, %d\n", sizeof(con)     , GLZ_CONS_SIZE);
    printf("table:   %zd, %d\n", sizeof(tbl)     , GLZ_TABLE_SIZE);
    printf("closure: %zd, %d\n", sizeof(closure) , GLZ_CLOSURE_SIZE);
    printf("regexp:  %zd, %d\n", sizeof(reg)     , GLZ_REGEXP_SIZE);
    printf("thread:  %zd, %d\n", sizeof(thread)  , GLZ_THREAD_SIZE);
    printf("tagged:  %zd, %d\n", sizeof(tagged)  , GLZ_TAGGED_SIZE);
    printf("obj:     %zd, %d\n", sizeof(obj)     , GLZ_SYMBOL_SIZE);
    printf("value:   %zd, %d\n", sizeof(nil)     , GLZ_VALUE_SIZE);

    glz_obj* s1 = (glz_obj*)symbol_new("s1", 2);
    glz_obj* s2 = (glz_obj*)symbol_new("s1", 2);
    glz_obj* s3 = (glz_obj*)symbol_new("s2", 2);

    printf("symbol1:   %d\n", s1 == s2);
    printf("symbol2:   %d\n", s1 != s3);

    glz_string* s4 = &(s1->symbol.name);

    printf("symbol3:   %s\n", (const char*)(s4+1));

    glz_obj* s5 = (glz_obj*)s4;

    printf("symbol4:   %d\n", is_string(s5));
    printf("symbol5:   %s\n", (const char*)((&(s5->string)) + 1));

/*

    glz_obj* a = (glz_obj*)symbol_new("a", 1);
    glz_obj* b = (glz_obj*)symbol_new("b", 1);
    glz_obj* c = (glz_obj*)symbol_new("c", 1);
    glz_obj* d = (glz_obj*)symbol_new("d", 1);
    glz_obj* e = (glz_obj*)symbol_new("e", 1);
    glz_obj* f = (glz_obj*)symbol_new("f", 1);
    glz_obj* g = (glz_obj*)symbol_new("g", 1);
    glz_obj* h = (glz_obj*)symbol_new("h", 1);
    glz_obj* i = (glz_obj*)symbol_new("i", 1);

    printf("symbol6:   %d\n", a != b);
    printf("symbol7:   %d\n", a != c);
    printf("symbol8:    %s\n", (const char*)(&(a->symbol.name) + 1));
    printf("symbol9:    %s\n", (const char*)(&(b->symbol.name) + 1));
    printf("symbol10:   %s\n", (const char*)(&(c->symbol.name) + 1));
    printf("symbol11:   %s\n", (const char*)(&(d->symbol.name) + 1));
    printf("symbol12:   %s\n", (const char*)(&(e->symbol.name) + 1));
    printf("symbol13:   %s\n", (const char*)(&(f->symbol.name) + 1));
    printf("symbol14:   %s\n", (const char*)(&(g->symbol.name) + 1));
    printf("symbol15:   %s\n", (const char*)(&(h->symbol.name) + 1));
    printf("symbol16:   %s\n", (const char*)(&(i->symbol.name) + 1));
*/

    return 0;
}

int fdprintf(int fd, const char *fmt, ...) {
    va_list ap;
    FILE *f = fdopen(fd, "a");
    int rc;

    va_start(ap, fmt);
    rc = vfprintf(f, fmt, ap);
    fclose(f);
    va_end(ap);
    return rc;
}

static void glz_init()
{
    string_init();
    object_init();
    reader_init();
}

static void glz_exit()
{
    string_exit();
}

int main(int argc, char* argv[])
{
    glz_init();

    obj_tests();

    printf( "glaze version " GLZ_VERSION " compiled at %s %s\n", __DATE__, __TIME__ );

    glz_obj* s_eof  = (glz_obj*)symbol_new("", 0);

retry:
    while (1) {
        fprintf(stdout, "> ");
        fflush(stdout);
        glz_obj* x = reader_read_fp(stdin);
        if (x == s_eof) break;
        object_print_fp(x, stdout);
        fprintf(stdout, "\n");
    }

    glz_exit();
    return 0;
}
