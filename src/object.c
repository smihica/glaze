#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gc.h"
#include "object.h"
#include "string.h"
#include "symbol.h"
#include "cons.h"

glz_obj* glz_nil;
glz_obj* glz_t;

void object_init()
{
    glz_nil = (glz_obj*)value_new(GLZ_TT_NIL);
    glz_t   = (glz_obj*)value_new(GLZ_TT_T);
}

inline size_t object_type_size_of(uint8_t tt)
{
    switch (tt) {
    case GLZ_TT_NIL:
    case GLZ_TT_T:
    case GLZ_TT_FIXNUM:
    case GLZ_TT_FLOATNUM:
        return GLZ_VALUE_SIZE;
    case GLZ_TT_FRACTION:
    case GLZ_TT_IMAGINARY:
        return GLZ_NUMBER_SIZE;
    case GLZ_TT_SSTRING:
    case GLZ_TT_LSTRING:
        return GLZ_STRING_SIZE;
    case GLZ_TT_SYMBOL:
        return GLZ_SYMBOL_SIZE;
    case GLZ_TT_CONS:
        return GLZ_CONS_SIZE;
    case GLZ_TT_TABLE:
        return GLZ_TABLE_SIZE;
    case GLZ_TT_CLOSURE:
        return GLZ_CLOSURE_SIZE;
    case GLZ_TT_REGEXP:
        return GLZ_REGEXP_SIZE;
    case GLZ_TT_THREAD:
        return GLZ_THREAD_SIZE;
    case GLZ_TT_TAGGED:
        return GLZ_TAGGED_SIZE;
    default:
        return GLZ_HEADER_SIZE;
    }
}

glz_obj* object_new(uint8_t tt, size_t trails_size, glz_obj **next, int offset)
{
    size_t size = object_type_size_of(tt) + trails_size;
    void*  raw = gc_newblock(NULL, 0, size);
    glz_obj* o = (glz_obj*)(raw + offset);
    o->header.marked = 0;
    o->header.tt     = tt;
    o->header.next   = 0;
    return o;
}

glz_value* value_new(uint8_t tt)
{
    void* raw = gc_newblock(NULL, 0, GLZ_VALUE_SIZE);
    glz_value* v = (glz_value*)(raw);
    v->tt = tt;
    return v;
}

glz_string* object_string_new(const char* str, size_t l, uint32_t hash)
{
    uint8_t     tt = (l < (GLZ_SSTRING_LIMIT-1)) ? GLZ_TT_SSTRING : GLZ_TT_LSTRING;
    glz_string* s;
    if (tt == GLZ_TT_SSTRING) {
        glz_symbol* wrapper = &(object_new(GLZ_TT_SYMBOL, (sizeof(char) * (l + 1)), NULL, 0)->symbol);
        s = &(wrapper->name);
        s->marked = wrapper->marked;
        s->tt     = tt;
        s->next   = wrapper->next;
    } else {
        s = &(object_new(tt, (sizeof(char) * (l + 1)), NULL, 0)->string);
    }

    s->size = (uint32_t)l;
    s->hash = hash;
    memcpy(s+1, str, l*sizeof(char));
    ((char *)(s+1))[l] = '\0';

    return s;
}

glz_cons* object_cons_new(glz_obj* car, glz_obj* cdr)
{
    glz_cons* cons = &(object_new(GLZ_TT_CONS, 0, NULL, 0)->cons);
    cons->car = car;
    cons->cdr = cdr;
    return cons;
}

ssize_t object_print_fp(glz_obj* o, FILE* fp)
{
    uint8_t tt = o->header.tt;
    switch(tt) {
    case GLZ_TT_NIL:
        return (ssize_t)fprintf(fp, "nil");
    case GLZ_TT_T:
        return (ssize_t)fprintf(fp, "t");
    case GLZ_TT_NUMBER:
        return 0; // TODO
    case GLZ_TT_STRING:
        return string_print_fp(&(o->string), fp);
    case GLZ_TT_SYMBOL:
        return symbol_print_fp(&(o->symbol), fp);
    case GLZ_TT_CONS:
        return cons_print_fp(&(o->cons), fp);
    case GLZ_TT_TABLE:
    case GLZ_TT_CLOSURE:
    case GLZ_TT_REGEXP:
    case GLZ_TT_THREAD:
    case GLZ_TT_TAGGED:
    default:
        return 0; // TODO
    }
}
