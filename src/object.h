#ifndef GLAZE__OBJECT_H_
#define GLAZE__OBJECT_H_

#include <stdio.h>
#include <stdint.h>

/** Type Tag Spec

 3bit - local tag  (short string long string ...) 8 pattern
 5bit - global tag (symbol string ...) 32 pattern

         |local| global  |
 uint8_t |0|0|0|0|0|0|0|0|
 */

/** global type tags */
#define GLZ_TT_NIL            0
#define GLZ_TT_T              1
#define GLZ_TT_NUMBER         2
#define GLZ_TT_STRING         3
#define GLZ_TT_SYMBOL         4
#define GLZ_TT_CONS           5
#define GLZ_TT_TABLE          6
#define GLZ_TT_CLOSURE        7
#define GLZ_TT_REGEXP         8
#define GLZ_TT_THREAD         9
#define GLZ_TT_TAGGED         10


#define GLZ_CREATE_LOCAL_TT(global_tag, local_tag) \
    (global_tag | (local_tag << 5))

/** local types of number */
#define GLZ_TT_FIXNUM         GLZ_CREATE_LOCAL_TT (GLZ_TT_NUMBER, 0)
#define GLZ_TT_FLOATNUM       GLZ_CREATE_LOCAL_TT (GLZ_TT_NUMBER, 1)
#define GLZ_TT_FRACTION       GLZ_CREATE_LOCAL_TT (GLZ_TT_NUMBER, 2)
#define GLZ_TT_IMAGINARY      GLZ_CREATE_LOCAL_TT (GLZ_TT_NUMBER, 3)
/** local types of string */
#define GLZ_TT_SSTRING        GLZ_CREATE_LOCAL_TT (GLZ_TT_STRING, 0)
#define GLZ_TT_LSTRING        GLZ_CREATE_LOCAL_TT (GLZ_TT_STRING, 1)

#define GLZ_MASC_LOCAL_TT(tt) ((tt) & 31)

/** typetag_check */
#define is_nil(o)             ((o)->header.tt == GLZ_TT_NIL)
#define is_t(o)               ((o)->header.tt == GLZ_TT_T)
#define is_number(o)          (GLZ_MASC_LOCAL_TT((o)->header.tt) == GLZ_TT_NUMBER)
#define is_string(o)          (GLZ_MASC_LOCAL_TT((o)->header.tt) == GLZ_TT_STRING)
#define is_symbol(o)          ((o)->header.tt == GLZ_TT_SYMBOL)
#define is_cons(o)            ((o)->header.tt == GLZ_TT_CONS)
#define is_table(o)           ((o)->header.tt == GLZ_TT_TABLE)
#define is_closure(o)         ((o)->header.tt == GLZ_TT_CLOSURE)
#define is_regexp(o)          ((o)->header.tt == GLZ_TT_REGEXP)
#define is_thread(o)          ((o)->header.tt == GLZ_TT_THREAD)
#define is_tagged(o)          ((o)->header.tt == GLZ_TT_TAGGED)
#define is_fixnum(o)          ((o)->header.tt == GLZ_TT_FIXNUM)
#define is_floatnum(o)        ((o)->header.tt == GLZ_TT_FLOATNUM)
#define is_fraction(o)        ((o)->header.tt == GLZ_TT_FRACTION)
#define is_imaginary(o)       ((o)->header.tt == GLZ_TT_IMAGINARY)
#define is_sstring(o)         ((o)->header.tt == GLZ_TT_SSTRING)
#define is_lstring(o)         ((o)->header.tt == GLZ_TT_LSTRING)

typedef union glz_obj glz_obj;

#define GLZ_HEADER \
    uint8_t   tt;         \
    uint8_t   marked;     \
    glz_obj*  next;

typedef struct glz_header {
    GLZ_HEADER
} glz_header;

typedef struct glz_number {
    GLZ_HEADER
} glz_number;

typedef struct glz_string {
    GLZ_HEADER
    uint32_t size;
    uint32_t hash;
} glz_string;

typedef struct glz_symbol {
    GLZ_HEADER
    glz_string name;
} glz_symbol;

typedef struct glz_cons {
    GLZ_HEADER
    glz_obj* car;
    glz_obj* cdr;
} glz_cons;

typedef struct glz_table {
    GLZ_HEADER
} glz_table;

typedef struct glz_closure {
    GLZ_HEADER
} glz_closure;

typedef struct glz_regexp {
    GLZ_HEADER
} glz_regexp;

typedef struct glz_thread {
    GLZ_HEADER
} glz_thread;

typedef struct glz_tagged {
    GLZ_HEADER
} glz_tagged;

#define GLZ_HEADER_SIZE    (16)
#define GLZ_NUMBER_SIZE    (GLZ_HEADER_SIZE + 0)
#define GLZ_STRING_SIZE    (GLZ_HEADER_SIZE + 8)
#define GLZ_SYMBOL_SIZE    (GLZ_HEADER_SIZE + GLZ_STRING_SIZE)
#define GLZ_CONS_SIZE      (GLZ_HEADER_SIZE + 8 + 8)
#define GLZ_TABLE_SIZE     (GLZ_HEADER_SIZE + 0)
#define GLZ_CLOSURE_SIZE   (GLZ_HEADER_SIZE + 0)
#define GLZ_REGEXP_SIZE    (GLZ_HEADER_SIZE + 0)
#define GLZ_THREAD_SIZE    (GLZ_HEADER_SIZE + 0)
#define GLZ_TAGGED_SIZE    (GLZ_HEADER_SIZE + 0)

#define GLZ_VALUE_SIZE    (16)
#define GLZ_FIXNUM_SIZE   8
#define GLZ_FLOATNUM_SIZE 8

typedef double  glz_subr;
typedef int64_t glz_fixnum;
typedef double  glz_floatnum;

typedef struct glz_value {
    uint8_t tt;
    union {
        glz_obj*       obj;
        glz_subr       subr;
        glz_fixnum     fixnum;
        glz_floatnum   floatnum;
    } v;
} glz_value;

union glz_obj {
    glz_header    header; /* common header */
    glz_number    number;
    glz_symbol    symbol;
    glz_cons      cons;
    glz_string    string;
    glz_table     table;
    glz_closure   closure;
    glz_regexp    regexp;
    glz_tagged    tagged;
    glz_value     value;
};

#define GLZ_SSTRING_LIMIT 64
#define GLZ_SYMBOL_LIMIT  GLZ_SSTRING_LIMIT

void object_init();
extern inline size_t object_type_size_of(uint8_t tt);
glz_obj* object_new(uint8_t tt, size_t trails_size, glz_obj **next, int offset);
glz_value* value_new(uint8_t tt);
glz_string* object_string_new(const char* str, size_t l, uint32_t hash);
glz_cons*   object_cons_new(glz_obj* car, glz_obj* cdr);
ssize_t object_print_fp(glz_obj* o, FILE* fp);

extern glz_obj* glz_nil;
extern glz_obj* glz_t;

#endif // GLAZE__OBJECT_H_
